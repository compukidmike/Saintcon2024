#include "ir_comm.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/rmt_tx.h"
#include "driver/rmt_rx.h"
#include "ir_nec_encoder.h"

static const char *TAG = "ir_comm";

#define IR_RESOLUTION_HZ  1000000 // 1MHz resolution, 1 tick = 1us
#define NEC_DECODE_MARGIN 200     // Tolerance for parsing RMT symbols into bit stream
#ifdef IR_COMM_DEBUG
    #define IR_COMM_DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
    #define IR_COMM_DEBUG_PRINT(...)
#endif

/**
 * @brief NEC timing spec
 */
#define NEC_LEADING_CODE_DURATION_0 9000
#define NEC_LEADING_CODE_DURATION_1 4500
#define NEC_PAYLOAD_ZERO_DURATION_0 560
#define NEC_PAYLOAD_ZERO_DURATION_1 560
#define NEC_PAYLOAD_ONE_DURATION_0  560
#define NEC_PAYLOAD_ONE_DURATION_1  1690
#define NEC_REPEAT_CODE_DURATION_0  9000
#define NEC_REPEAT_CODE_DURATION_1  2250

static rmt_channel_handle_t rmt_tx_channel = NULL;
static rmt_channel_handle_t rmt_rx_channel = NULL;
static rmt_encoder_handle_t nec_encoder    = NULL;
static QueueHandle_t receive_queue         = NULL;
static ir_rx_callback_t user_rx_callback   = NULL;
static TaskHandle_t rx_task_handle         = NULL;

static uint16_t s_nec_code_address;
static uint16_t s_nec_code_command;

// Function prototypes
static void ir_rx_task(void *arg);

/**
 * @brief Check whether a signal duration is within the expected range
 */
static bool nec_check_in_range(uint32_t signal_duration, uint32_t spec_duration) {
    return (signal_duration < (spec_duration + NEC_DECODE_MARGIN)) && (signal_duration > (spec_duration - NEC_DECODE_MARGIN));
}

/**
 * @brief Check whether a RMT symbol represents NEC logic zero
 */
static bool nec_parse_logic0(rmt_symbol_word_t *rmt_nec_symbols) {
    return nec_check_in_range(rmt_nec_symbols->duration0, NEC_PAYLOAD_ZERO_DURATION_0) &&
           nec_check_in_range(rmt_nec_symbols->duration1, NEC_PAYLOAD_ZERO_DURATION_1);
}

/**
 * @brief Check whether a RMT symbol represents NEC logic one
 */
static bool nec_parse_logic1(rmt_symbol_word_t *rmt_nec_symbols) {
    return nec_check_in_range(rmt_nec_symbols->duration0, NEC_PAYLOAD_ONE_DURATION_0) &&
           nec_check_in_range(rmt_nec_symbols->duration1, NEC_PAYLOAD_ONE_DURATION_1);
}

/**
 * @brief Decode RMT symbols into NEC address and command
 */
static bool nec_parse_frame(rmt_symbol_word_t *rmt_nec_symbols) {
    rmt_symbol_word_t *cur  = rmt_nec_symbols;
    uint16_t address        = 0;
    uint16_t command        = 0;
    bool valid_leading_code = nec_check_in_range(cur->duration0, NEC_LEADING_CODE_DURATION_0) &&
                              nec_check_in_range(cur->duration1, NEC_LEADING_CODE_DURATION_1);
    if (!valid_leading_code) {
        return false;
    }
    cur++;
    for (int i = 0; i < 16; i++) {
        if (nec_parse_logic1(cur)) {
            address |= 1 << i;
        } else if (nec_parse_logic0(cur)) {
            address &= ~(1 << i);
        } else {
            return false;
        }
        cur++;
    }
    for (int i = 0; i < 16; i++) {
        if (nec_parse_logic1(cur)) {
            command |= 1 << i;
        } else if (nec_parse_logic0(cur)) {
            command &= ~(1 << i);
        } else {
            return false;
        }
        cur++;
    }
    s_nec_code_address = address;
    s_nec_code_command = command;
    return true;
}

/**
 * @brief Check whether the RMT symbols represent NEC repeat code
 */
static bool nec_parse_frame_repeat(rmt_symbol_word_t *rmt_nec_symbols) {
    return nec_check_in_range(rmt_nec_symbols->duration0, NEC_REPEAT_CODE_DURATION_0) &&
           nec_check_in_range(rmt_nec_symbols->duration1, NEC_REPEAT_CODE_DURATION_1);
}

static void parse_nec_frame(rmt_symbol_word_t *rmt_nec_symbols, size_t symbol_num) {
    IR_COMM_DEBUG_PRINT("NEC frame start -----\n");
    for (size_t i = 0; i < symbol_num; i++) {
        IR_COMM_DEBUG_PRINT("{%d:%d},{%d:%d}\n", rmt_nec_symbols[i].level0, rmt_nec_symbols[i].duration0,
                            rmt_nec_symbols[i].level1, rmt_nec_symbols[i].duration1);
    }
    IR_COMM_DEBUG_PRINT("NEC frame end -----\n");

    // Decode RMT symbols
    switch (symbol_num) {
        case 34: // NEC normal frame
            if (nec_parse_frame(rmt_nec_symbols)) {
                IR_COMM_DEBUG_PRINT("NEC normal - Address: 0x%04x, Command: 0x%04x\n", s_nec_code_address, s_nec_code_command);
                if (user_rx_callback) {
                    user_rx_callback(s_nec_code_address, s_nec_code_command);
                }
            }
            break;
        case 2: // NEC repeat frame
            if (nec_parse_frame_repeat(rmt_nec_symbols)) {
                IR_COMM_DEBUG_PRINT("NEC repeat - Address: 0x%04x, Command: 0x%04x\n", s_nec_code_address, s_nec_code_command);
                if (user_rx_callback) {
                    user_rx_callback(s_nec_code_address, s_nec_code_command);
                }
            }
            break;
        default: IR_COMM_DEBUG_PRINT("Unknown NEC frame\n"); break;
    }
}

static bool rmt_rx_done_callback(rmt_channel_handle_t channel, const rmt_rx_done_event_data_t *edata, void *user_data) {
    (void)channel;
    (void)user_data;
    BaseType_t high_task_wakeup = pdFALSE;
    xQueueSendFromISR(receive_queue, edata, &high_task_wakeup);
    return high_task_wakeup == pdTRUE;
}

esp_err_t ir_init(ir_rx_callback_t rx_callback) {
    ESP_LOGI(TAG, "Initializing IR component");

    // Store the user's callback
    user_rx_callback = rx_callback;

    // Create the receive queue
    receive_queue = xQueueCreate(1, sizeof(rmt_rx_done_event_data_t));
    if (!receive_queue) {
        return ESP_ERR_NO_MEM;
    }

    // Initialize RX channel
    rmt_rx_channel_config_t rx_channel_cfg = {
        .clk_src           = RMT_CLK_SRC_DEFAULT,
        .resolution_hz     = IR_RESOLUTION_HZ,
        .mem_block_symbols = 64, // Number of symbols one channel can store at a time
        .gpio_num          = CONFIG_IR_RX_GPIO,
    };
    ESP_ERROR_CHECK(rmt_new_rx_channel(&rx_channel_cfg, &rmt_rx_channel));

    rmt_rx_event_callbacks_t cbs = {
        .on_recv_done = rmt_rx_done_callback,
    };
    ESP_ERROR_CHECK(rmt_rx_register_event_callbacks(rmt_rx_channel, &cbs, NULL));

    // Initialize TX channel
    rmt_tx_channel_config_t tx_channel_cfg = {
        .clk_src           = RMT_CLK_SRC_DEFAULT,
        .resolution_hz     = IR_RESOLUTION_HZ,
        .mem_block_symbols = 64, // Number of symbols one channel can store at a time
        .trans_queue_depth = 4,  // Allowed transactions pending in background
        .gpio_num          = CONFIG_IR_TX_GPIO,
    };
    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_channel_cfg, &rmt_tx_channel));

    rmt_carrier_config_t carrier_cfg = {
        .duty_cycle   = 0.33,
        .frequency_hz = 38000, // NEC protocol carrier frequency - 38kHz
    };
    ESP_ERROR_CHECK(rmt_apply_carrier(rmt_tx_channel, &carrier_cfg));

    ir_nec_encoder_config_t nec_encoder_cfg = {
        .resolution = IR_RESOLUTION_HZ,
    };
    ESP_ERROR_CHECK(rmt_new_ir_nec_encoder(&nec_encoder_cfg, &nec_encoder));

    ESP_LOGI(TAG, "IR component initialized successfully");
    return ESP_OK;
}

esp_err_t ir_enable_rx() {
    if (rx_task_handle != NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "Enabling RX channel");
    ESP_ERROR_CHECK(rmt_enable(rmt_rx_channel));

    // Start the RX task
    xTaskCreate(ir_rx_task, "ir_rx_task", 4096, NULL, 10, &rx_task_handle);
    return ESP_OK;
}

esp_err_t ir_disable_rx() {
    if (rx_task_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "Disabling RX channel");
    vTaskDelete(rx_task_handle);
    rx_task_handle = NULL;
    ESP_ERROR_CHECK(rmt_disable(rmt_rx_channel));
    return ESP_OK;
}

esp_err_t ir_enable_tx() {
    ESP_LOGI(TAG, "Enabling TX channel");
    return rmt_enable(rmt_tx_channel);
}

esp_err_t ir_disable_tx() {
    ESP_LOGI(TAG, "Disabling TX channel");
    return rmt_disable(rmt_tx_channel);
}

static void ir_rx_task(void *_arg) {
    (void)_arg;
    rmt_symbol_word_t raw_symbols[64];
    rmt_rx_done_event_data_t rx_data;
    rmt_receive_config_t receive_config = {
        .signal_range_min_ns = 1250, // Shortest duration for NEC is 560us, 1250ns < 560us valid signal won't be considered noise
        .signal_range_max_ns = 12000000, // Longest duration for NEC is 9000us, 12000000ns > 9000us so receive won't stop early
    };

    ESP_LOGI(TAG, "Starting RX task");
    while (1) {
        ESP_ERROR_CHECK(rmt_receive(rmt_rx_channel, raw_symbols, sizeof(raw_symbols), &receive_config));
        if (xQueueReceive(receive_queue, &rx_data, portMAX_DELAY) == pdPASS) {
            if (user_rx_callback) {
                parse_nec_frame(rx_data.received_symbols, rx_data.num_symbols);
            }
        }
    }
}

esp_err_t ir_transmit(uint16_t address, uint16_t command) {
    const ir_nec_scan_code_t scan_code = {
        .address = address,
        .command = command,
    };
    rmt_transmit_config_t transmit_config = {
        .loop_count = 0, // Single transmission, no loop
    };

    ESP_LOGD(TAG, "Transmitting IR code - Address: 0x%04x, Command: 0x%04x", address, command);
    return rmt_transmit(rmt_tx_channel, nec_encoder, &scan_code, sizeof(scan_code), &transmit_config);
}
