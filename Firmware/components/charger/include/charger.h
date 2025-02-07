#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "esp_err.h"

// 7-bit I2C address for the BQ24296M
#define BQ24296M_I2C_ADDRESS 0x6B

// Event group for the charger
extern EventGroupHandle_t charger_event_group;

// Queues for the charger status and faults
extern QueueHandle_t charger_status_queue;
extern QueueHandle_t charger_fault_queue;

// Event group bits
#define CHARGER_EVENT BIT0
#define CHARGER_FAULT BIT1

// Queue size
#define CHARGER_QUEUE_SIZE 10

// BQ24296M Registers
typedef enum {
    INPUT_SRC_CTRL           = 0x00,
    POWER_ON_CONFIG          = 0x01,
    CHARGE_CURRENT_CTRL      = 0x02,
    PRECHG_TERM_CURRENT_CTRL = 0x03,
    CHARGE_VOLTAGE_CTRL      = 0x04,
    CHARGE_TERM_TIMER_CTRL   = 0x05,
    BOOST_THERMAL_CTRL       = 0x06,
    MISC_OPERATION_CTRL      = 0x07,
    SYSTEM_STATUS            = 0x08,
    NEW_FAULT                = 0x09,
    VENDOR_STATUS            = 0x0A,
} bq24296m_register_t;

/*
 * INPUT_SRC_CTRL Register definitions
 */

// IINLIM Value (Input Current Limit)
typedef enum {
    IINLIM_100mA  = 0b000,
    IINLIM_150mA  = 0b001,
    IINLIM_500mA  = 0b010,
    IINLIM_900mA  = 0b011,
    IINLIM_1000mA = 0b100,
    IINLIM_1500mA = 0b101,
    IINLIM_2000mA = 0b110,
    IINLIM_3000mA = 0b111,
} charger_iinlim_t;

// VINDPM Value (Input Voltage Limit)
//   Offsets from: 3.88V
typedef enum {
    VINDPM_OFS_80mV  = 0b0001,
    VINDPM_OFS_160mV = 0b0010,
    VINDPM_OFS_320mV = 0b0100,
    VINDPM_OFS_640mV = 0b1000,
    VINDPM_3880mV    = 0b0000,
    VINDPM_3960mV    = 0b0001,
    VINDPM_4040mV    = 0b0010,
    VINDPM_4120mV    = 0b0011,
    VINDPM_4200mV    = 0b0100,
    VINDPM_4280mV    = 0b0101,
    VINDPM_4360mV    = 0b0110, // Default
    VINDPM_4440mV    = 0b0111,
    VINDPM_4520mV    = 0b1000,
    VINDPM_4600mV    = 0b1001,
    VINDPM_4680mV    = 0b1010,
    VINDPM_4760mV    = 0b1011,
    VINDPM_4840mV    = 0b1100,
    VINDPM_4920mV    = 0b1101,
    VINDPM_5000mV    = 0b1110,
    VINDPM_5080mV    = 0b1111, // Maximum
} charger_vindpm_t;

// Input Source Control Register (RW)
typedef union {
    uint8_t raw;
    struct {
        charger_iinlim_t iinlim : 3; // PSEL = Lo : 3A (111), PSEL = Hi : 100mA (000) (OTG pin = Lo) or 500mA (010) (OTG pin = Hi)
        charger_vindpm_t vindpm : 4; // Default: 4360mV (0110)
        bool en_hiz : 1;             // Hi-Z Mode Enable ([0]: Disable, 1: Enable)
    };
} charger_input_src_ctrl_t;

/*
 * POWER_ON_CONFIG Register definitions
 */

// BOOST_LIM Value (Boost Mode Current Limit)
typedef enum {
    BOOST_LIM_1000mA = 0,
    BOOST_LIM_1500mA = 1 // Default
} charger_boost_lim_t;

// SYS_MIN Value (Minimum System Voltage)
//   Offsets from: 3.0V
typedef enum {
    SYS_MIN_VOLT_OFS_100mV = 0b001,
    SYS_MIN_VOLT_OFS_200mV = 0b010,
    SYS_MIN_VOLT_OFS_400mV = 0b100,
    SYS_MIN_VOLT_3000mV    = 0b000,
    SYS_MIN_VOLT_3100mV    = 0b001,
    SYS_MIN_VOLT_3200mV    = 0b010,
    SYS_MIN_VOLT_3300mV    = 0b011,
    SYS_MIN_VOLT_3400mV    = 0b100,
    SYS_MIN_VOLT_3500mV    = 0b101, // Default
    SYS_MIN_VOLT_3600mV    = 0b110,
    SYS_MIN_VOLT_3700mV    = 0b111, // Maximum
} charger_sys_min_t;

// Power-On Configuration Register (RW)
typedef union {
    uint8_t raw;
    struct {
        charger_boost_lim_t boost_lim : 1; // Boost Mode Current Limit (0: 1000mA, [1]: 1500mA)
        charger_sys_min_t sys_min : 3;     // Minimum System Voltage Limit - Default: 3500mV (101)
        bool chg_config : 1;               // Charge Configuration (0: Disable, [1]: Enable)
        bool otg_config : 1;               // OTG Configuration ([0]: Disable, 1: Enable)
        bool wdt_reset : 1;                // I²C Watchdog Timer Reset ([0]: Normal, 1: Reset)
        bool reg_reset : 1;                // Register Reset ([0]: Keep, 1: Reset)
    };
} charger_power_on_config_t;

/*
 * CHARGE_CURRENT_CTRL Register definitions
 */

// BCOLD Value (Battery Cold Threshold)
typedef enum {
    BCOLD_76PERCENT_REGN = 0, // -10°C or 76% of REGN (internal regulator voltage) - Default
    BCOLD_79PERCENT_REGN = 1, // -20°C or 79% of REGN
} charger_bcold_t;

// ICHG Value (Fast Charge Current Limit)
//   Offsets from: 512mA
typedef enum {
    ICHG_OFS_64mA   = 0b000001,
    ICHG_OFS_128mA  = 0b000010,
    ICHG_OFS_256mA  = 0b000100,
    ICHG_OFS_512mA  = 0b001000,
    ICHG_OFS_1024mA = 0b010000,
    ICHG_OFS_2048mA = 0b100000,
    ICHG_512mA      = 0b000000,
    ICHG_576mA      = 0b000001,
    ICHG_640mA      = 0b000010,
    ICHG_704mA      = 0b000011,
    ICHG_768mA      = 0b000100,
    ICHG_832mA      = 0b000101,
    ICHG_896mA      = 0b000110,
    ICHG_960mA      = 0b000111,
    ICHG_1024mA     = 0b001000,
    ICHG_1088mA     = 0b001001,
    ICHG_1152mA     = 0b001010,
    ICHG_1216mA     = 0b001011,
    ICHG_1280mA     = 0b001100,
    ICHG_1344mA     = 0b001101,
    ICHG_1408mA     = 0b001110,
    ICHG_1472mA     = 0b001111,
    ICHG_1536mA     = 0b010000,
    ICHG_1600mA     = 0b010001,
    ICHG_1664mA     = 0b010010,
    ICHG_1728mA     = 0b010011,
    ICHG_1792mA     = 0b010100,
    ICHG_1856mA     = 0b010101,
    ICHG_1920mA     = 0b010110,
    ICHG_1984mA     = 0b010111,
    ICHG_2048mA     = 0b011000, // Default
    ICHG_2112mA     = 0b011001,
    ICHG_2176mA     = 0b011010,
    ICHG_2240mA     = 0b011011,
    ICHG_2304mA     = 0b011100,
    ICHG_2368mA     = 0b011101,
    ICHG_2432mA     = 0b011110,
    ICHG_2496mA     = 0b011111,
    ICHG_2560mA     = 0b100000,
    ICHG_2624mA     = 0b100001,
    ICHG_2688mA     = 0b100010,
    ICHG_2752mA     = 0b100011,
    ICHG_2816mA     = 0b100100,
    ICHG_2880mA     = 0b100101,
    ICHG_2944mA     = 0b100110,
    ICHG_3008mA     = 0b100111, // Maximum
} charger_ichg_t;

// Charge Current Control Register (RW)
typedef union {
    uint8_t raw;
    struct {
        bool force_20pct : 1;      // Force ICHG as 20% Fast Charge Current ([0]: Disable, 1: Enable)
        charger_bcold_t bcold : 1; // Battery Cold Threshold ([0]: -10°C or 76% of REGN, 1: -20°C or 79% of REGN)
        charger_ichg_t ichg : 6;   // Default: 2048mA (011000)
    };
} charger_charge_current_ctrl_t;

/*
 * PRECHG_TERM_CURRENT_CTRL Register definitions
 */

// ITERM Value (Termination Current)
//   Offsets from: 128mA
typedef enum {
    ITERM_OFS_128mA = 0b001,
    ITERM_OFS_256mA = 0b010,
    ITERM_OFS_512mA = 0b100,
    ITERM_128mA     = 0b000,
    ITERM_256mA     = 0b001, // Default
    ITERM_384mA     = 0b010,
    ITERM_512mA     = 0b011,
    ITERM_768mA     = 0b100,
    ITERM_896mA     = 0b101,
    ITERM_1024mA    = 0b110, // Maximum
} charger_iterm_t;

// IPRECHG Value (Pre-Charge Current)
typedef enum {
    IPRECHG_128mA  = 0b0001, // Default
    IPRECHG_256mA  = 0b0010,
    IPRECHG_384mA  = 0b0011,
    IPRECHG_512mA  = 0b0100,
    IPRECHG_768mA  = 0b0101,
    IPRECHG_896mA  = 0b0110,
    IPRECHG_1024mA = 0b0111,
    IPRECHG_1152mA = 0b1000,
    IPRECHG_1280mA = 0b1001,
    IPRECHG_1408mA = 0b1010,
    IPRECHG_1536mA = 0b1011,
    IPRECHG_1664mA = 0b1100,
    IPRECHG_1792mA = 0b1101,
    IPRECHG_1920mA = 0b1110,
    IPRECHG_2048mA = 0b1111,
} charger_iprechg_t;

// Pre-Charge/Termination Current Control Register (RW)
typedef union {
    uint8_t raw;
    struct {
        charger_iterm_t iterm : 3;     // Default: 128mA (001)
        uint8_t _reserved0 : 1;        //
        charger_iprechg_t iprechg : 4; // Default: 128mA (0001)
    };
} charger_prechg_term_current_ctrl_t;

/*
 * CHARGE_VOLTAGE_CTRL Register definitions
 */

// VRECHG Value (Battery Recharge Threshold)
typedef enum {
    VRECHG_100mV = 0, // Default
    VRECHG_300mV = 1,
} charger_vrechg_t;

// BATLOWV Value (Battery Low Voltage)
typedef enum {
    BATLOWV_2800mV = 0,
    BATLOWV_3000mV = 1, // Default
} charger_batlowv_t;

// VREG Value (Charge Voltage Limit)
//   Offsets from: 3.504V
typedef enum {
    VREG_OFS_16mV  = 0b000001,
    VREG_OFS_32mV  = 0b000010,
    VREG_OFS_64mV  = 0b000100,
    VREG_OFS_128mV = 0b001000,
    VREG_OFS_256mV = 0b010000,
    VREG_OFS_512mV = 0b100000,
    VREG_3504mV    = 0b000000,
    VREG_3520mV    = 0b000001,
    VREG_3536mV    = 0b000010,
    VREG_3552mV    = 0b000011,
    VREG_3568mV    = 0b000100,
    VREG_3584mV    = 0b000101,
    VREG_3600mV    = 0b000110,
    VREG_3616mV    = 0b000111,
    VREG_3632mV    = 0b001000,
    VREG_3648mV    = 0b001001,
    VREG_3664mV    = 0b001010,
    VREG_3680mV    = 0b001011,
    VREG_3696mV    = 0b001100,
    VREG_3712mV    = 0b001101,
    VREG_3728mV    = 0b001110,
    VREG_3744mV    = 0b001111,
    VREG_3760mV    = 0b010000,
    VREG_3776mV    = 0b010001,
    VREG_3792mV    = 0b010010,
    VREG_3808mV    = 0b010011,
    VREG_3824mV    = 0b010100,
    VREG_3840mV    = 0b010101,
    VREG_3856mV    = 0b010110,
    VREG_3872mV    = 0b010111,
    VREG_3888mV    = 0b011000,
    VREG_3904mV    = 0b011001,
    VREG_3920mV    = 0b011010,
    VREG_3936mV    = 0b011011,
    VREG_3952mV    = 0b011100,
    VREG_3968mV    = 0b011101,
    VREG_3984mV    = 0b011110,
    VREG_4000mV    = 0b011111,
    VREG_4016mV    = 0b100000,
    VREG_4032mV    = 0b100001,
    VREG_4048mV    = 0b100010,
    VREG_4064mV    = 0b100011,
    VREG_4080mV    = 0b100100,
    VREG_4096mV    = 0b100101,
    VREG_4112mV    = 0b100110,
    VREG_4128mV    = 0b100111,
    VREG_4144mV    = 0b101000,
    VREG_4160mV    = 0b101001,
    VREG_4176mV    = 0b101010,
    VREG_4192mV    = 0b101011,
    VREG_4208mV    = 0b101100, // Default
    VREG_4224mV    = 0b101101,
    VREG_4240mV    = 0b101110,
    VREG_4256mV    = 0b101111,
    VREG_4272mV    = 0b110000,
    VREG_4288mV    = 0b110001,
    VREG_4304mV    = 0b110010,
    VREG_4320mV    = 0b110011,
    VREG_4336mV    = 0b110100,
    VREG_4352mV    = 0b110101,
    VREG_4368mV    = 0b110110,
    VREG_4384mV    = 0b110111,
    VREG_4400mV    = 0b111000, // Maximum
} charger_vreg_t;

// Charge Voltage Control Register (RW)
typedef union {
    uint8_t raw;
    struct {
        charger_vrechg_t vrechg : 1;   // Battery Recharge Threshold ([0]: 100mV, 1: 300mV)
        charger_batlowv_t batlowv : 1; // Battery Low Voltage (0: 2800mV, [1]: 3000mV)
        charger_vreg_t vreg : 6;       // Default: 4208mV (101100)
    };
} charger_charge_voltage_ctrl_t;

/*
 * CHARGE_TERM_TIMER_CTRL Register definitions
 */

// CHG_TIMER Value (Charge Timer)
typedef enum {
    CHG_TIMER_5h  = 0b00,
    CHG_TIMER_8h  = 0b01,
    CHG_TIMER_12h = 0b10, // Default
    CHG_TIMER_20h = 0b11,
} charger_chg_timer_t;

// WATCHDOG Value (Watchdog Timer)
typedef enum {
    WATCHDOG_DISABLE = 0b00,
    WATCHDOG_40s     = 0b01, // Default
    WATCHDOG_80s     = 0b10,
    WATCHDOG_160s    = 0b11,
} charger_watchdog_timer_t;

/*
 * CHARGE_TERM_TIMER_CTRL Register definitions
 */

// Charge Termination/Timer Control Register (RW)
typedef union {
    uint8_t raw;
    struct {
        uint8_t _reserved0 : 1; //
        uint8_t chg_timer : 2;  // Charging Safety Timer - Default: 12h (10)
        bool en_timer : 1;      // Charging Safety Timer Enable (0: Disable, [1]: Enable)
        uint8_t watchdog : 2;   // Watchdog Timer - Default: 40s (01)
        uint8_t _reserved1 : 1; //
        bool en_term : 1;       // Charging Termination Enable (0: Disable, [1]: Enable)
    };
} charger_charge_term_timer_ctrl_t;

/*
 * BOOST_THERMAL_CTRL Register definitions
 */

// TREG Value (Thermal Regulation Threshold)
typedef enum {
    TREG_60DEG  = 0b00,
    TREG_80DEG  = 0b01,
    TREG_100DEG = 0b10,
    TREG_120DEG = 0b11, // Default
} charger_treg_t;

// BHOT Value (Boost Mode Hot Threshold)
typedef enum {
    BHOT_33PERCENT_REGN = 0b00, // 55°C or 33% of REGN - Default
    BHOT_36PERCENT_REGN = 0b01, // 60°C or 36% of REGN
    BHOT_30PERCENT_REGN = 0b10, // 65°C or 30% of REGN
    BHOT_DISABLE        = 0b11,
} charger_bhot_t;

// BOOSTV Value (Boost Mode Voltage)
typedef enum {
    BOOSTV_4550mV_OFFSET = 0b0000,
    BOOSTV_64mV          = 0b0001,
    BOOSTV_128mV         = 0b0010,
    BOOSTV_256mV         = 0b0100,
    BOOSTV_512mV         = 0b1000,
} charger_boostv_t;

// Boost Voltage/Thermal Regulation Control Register (RW)
typedef union {
    uint8_t raw;
    struct {
        charger_treg_t treg : 2;     // Thermal Regulation Threshold - Default: 120°C (11)
        charger_bhot_t bhot : 2;     // Boost Mode Hot Threshold - Default: 55°C or 33% of REGN (00)
        charger_boostv_t boostv : 4; // Boost Mode Voltage - Default: 4998mV (0111)
    };
} charger_boost_thermal_ctrl_t;

/*
 * MISC_OPERATION_CTRL Register definitions
 */

// INT_MASK Value (Interrupt Mask)
typedef enum {
    INT_MASK_NONE       = 0b00, // No INT on BAT_FAULT or CHRG_FAULT
    INT_MASK_CHRG_FAULT = 0b01, // INT during CHRG_FAULT
    INT_MASK_BAT_FAULT  = 0b10, // INT during BAT_FAULT
    INT_MASK_ALL        = 0b11, // INT during BAT_FAULT or CHRG_FAULT
} charger_int_mask_t;

// Miscellaneous Operation Control Register (RW)
typedef union {
    uint8_t raw;
    struct {
        charger_int_mask_t int_mask : 2; // Interrupt Mask (INT during fault) - Default: 11 - INT on faults
        uint8_t _reserved0 : 3;          //
        bool batfet_disable : 1;         // BATFET Disable ([0]: Enable, 1: Disable)
        bool tmr2x_en : 1;               // Slow safety timer by 2X during input DPM or thermal regulation - Default: true
        bool dpdm_en : 1;                // Force DPDM Detection ([0]: Disable, 1: Force when VBUS present)
    };
} charger_misc_operation_ctrl_t;

/*
 * SYSTEM_STATUS Register definitions
 */

// CHRG_STAT Value (Charge Status)
typedef enum {
    CHRG_STAT_NOT_CHARGING  = 0b00,
    CHRG_STAT_PRE_CHARGING  = 0b01,
    CHRG_STAT_FAST_CHARGING = 0b10,
    CHRG_STAT_CHARGE_DONE   = 0b11,
} charger_chrg_stat_t;

// VBUS_STAT Value (VBUS Status)
typedef enum {
    VBUS_STAT_UNKNOWN  = 0b00,
    VBUS_STAT_USB_HOST = 0b01,
    VBUS_STAT_ADAPTER  = 0b10,
    VBUS_STAT_USB_OTG  = 0b11,
} charger_vbus_stat_t;

// System Status Register (RO)
typedef union {
    uint8_t raw;
    struct {
        bool vsys_stat : 1;                // VSYSMIN Regulation (0: BAT > VSYSMIN, 1: BAT < VSYSMIN)
        bool therm_stat : 1;               // Thermal Regulation Status (0: Normal, 1: Thermal Regulation)
        bool pg_stat : 1;                  // Power Good Status (0: Not Power Good, 1: Power Good)
        bool dpm_stat : 1;                 // DPM Status (0: Not DPM, 1: VINDPM or IINDPM)
        charger_chrg_stat_t chrg_stat : 2; // Charge Status (00: Not Charging, 01: Pre-charge, 10: Fast Charge, 11: Charge
                                           // Termination)
        charger_vbus_stat_t vbus_stat : 2; // VBUS Status (00: Unknown, 01: USB Host, 10: Adapter Port, 11: OTG)
    };
} charger_system_status_t;

/*
 * NEW_FAULT Register definitions
 */

// NTC_FAULT Value (NTC Fault)
typedef enum {
    NTC_FAULT_NORMAL = 0b00,
    NTC_FAULT_COLD   = 0b01,
    NTC_FAULT_HOT    = 0b10,
} charger_ntc_fault_t;

// CHRG_FAULT Value (Charge Fault)
typedef enum {
    CHRG_FAULT_NORMAL  = 0b00,
    CHRG_FAULT_INPUT   = 0b01,
    CHRG_FAULT_THERMAL = 0b10,
    CHRG_FAULT_TIMER   = 0b11
} charger_chrg_fault_t;

// Fault Register (RO)
//     NOTES:
//       - Only supports single byte read.
//       - All fault register bits are latched fault. First time read clears previous fault and second read updates to any new
//         fault that is still present.
//       - When adapter is unplugged, input fault in CHRG_FAULT bits[5:4] is set to 01 once.
typedef union {
    uint8_t raw;
    struct {
        charger_ntc_fault_t ntc_fault : 2;   // NTC Fault (00: Normal, 01: Cold, 10: Hot)
        uint8_t _reserved0 : 1;              //
        bool bat_fault : 1;                  // Battery Fault (0: Normal, 1: Battery OVP)
        charger_chrg_fault_t chrg_fault : 2; // Charge Fault (00: Normal, 01: Input Fault, 10: Thermal Shutdown, 11: Charge Timer
                                             // Expired)
        bool otg_fault : 1; // OTG Fault (0: Normal, 1: OTG Fault - VBUS overload in OTG, VBUS OVP, low battery, etc.)
        bool watchdog : 1;  // Watchdog Fault (0: Normal, 1: Watchdog Timer Expired)
    };
} charger_new_fault_t;

/*
 * VENDOR_STATUS Register definitions
 */

// Vendor/Part/Revision Status Register (RO)
typedef union {
    uint8_t raw;
    struct {
        uint8_t rev : 3;        // Revision Number - Default: 000
        uint8_t _reserved0 : 2; //
        uint8_t pn : 3;         // Part Number - Default: 001 (bq24296M)
    };
} charger_vendor_status_t;

/**
 * @brief Initialize the BQ24296M device
 *
 * @return ESP_OK on success or an error code on failure
 */
esp_err_t charger_init();

/**
 * @brief Read a register from the charger
 *
 * @param reg The register to read
 * @param data Pointer to the data to read
 *
 * @return ESP_OK on success or an error code on failure
 */
esp_err_t charger_read_register(bq24296m_register_t reg, uint8_t *data);

/**
 * @brief Write a register to the charger
 *
 * @param reg The register to write
 * @param data The data to write
 *
 * @return ESP_OK on success or an error code on failure
 */
esp_err_t charger_write_register(bq24296m_register_t reg, uint8_t data);

/**
 * @brief Set the input voltage and current
 *
 * @param voltage The input voltage limit
 * @param current The input current limit
 *
 * @return ESP_OK on success or an error code on failure
 */
esp_err_t charger_set_input_power(uint8_t voltage, uint8_t current);

/**
 * @brief Enable or disable the charger
 *
 * @param enable
 * @return ESP_OK on success or an error code on failure
 */
esp_err_t charger_set_charge_enable(bool enable);

/**
 * @brief Set the minimum system voltage
 *
 * @param vsysmin The minimum system voltage
 * @return ESP_OK on success or an error code on failure
 */
esp_err_t charger_set_minimum_system_voltage(charger_sys_min_t vsysmin);

/**
 * @brief Set the charge current
 *
 * @param ichg The fast charge current limit (512mA - 3008mA, Default: 2048mA (011000))
 * @param force_20pct Force the charge current to 20% of the fast charge current instead of the full fast charge current (default:
 * false)
 * @return ESP_OK on success or an error code on failure
 */
esp_err_t charger_set_fast_charge_current(charger_ichg_t ichg, bool force_20pct);

/**
 * @brief Set the termination current
 *
 * @param iterm The termination current
 * @return ESP_OK on success or an error code on failure
 */
esp_err_t charger_set_termination_current(charger_iterm_t iterm);

/**
 * @brief Set the charge voltage limit
 *
 * @param vreg The charge voltage
 * @return ESP_OK on success or an error code on failure
 */
esp_err_t charger_set_charge_voltage_limit(charger_vreg_t vreg);

/**
 * @brief Read the charger status
 *
 * @param status Pointer to a status structure to fill
 * @return ESP_OK on success or an error code on failure
 */
esp_err_t charger_read_status(charger_system_status_t *status);

/**
 * @brief Read the charger faults
 *
 * @param faults Pointer to a faults structure to fill
 * @return ESP_OK on success or an error code on failure
 */
esp_err_t charger_read_faults(charger_new_fault_t *faults);

/**
 * @brief Read the charger revision/part number information
 *
 * @param vendor Pointer to a vendor structure to fill
 * @return ESP_OK on success or an error code on failure
 */
esp_err_t charger_read_vendor(charger_vendor_status_t *vendor);

/**
 * @brief Return the current charger status
 *
 * @return charger_system_status_t The current charger status
 */
charger_system_status_t charger_get_status();

/**
 * @brief Return the current charger faults
 *
 * @return charger_new_fault_t The current charger faults
 */
charger_new_fault_t charger_get_faults();

/**
 * @brief Reset the watchdog timer (POWER_ON_CONFIG register)
 *
 * @return ESP_OK on success or an error code on failure
 */
esp_err_t charger_reset_watchdog();

/**
 * @brief Set watchdog timer configuration
 *
 * @param timer The watchdog timer configuration
 * @return ESP_OK on success or an error code on failure
 */
esp_err_t charger_set_watchdog_timer(charger_watchdog_timer_t timer);

/**
 * @brief Set OTG mode to provide 5V on the VBUS pin
 *
 * @param enable Enable OTG mode
 * @return ESP_OK on success or an error code on failure
 */
esp_err_t charger_set_otg_mode(bool enable);

#ifdef __cplusplus
}
#endif
