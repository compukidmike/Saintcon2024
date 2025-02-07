#include <iostream>
#include <sys/socket.h>
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_ota_ops.h"
#include "esp_https_ota.h"
#include "esp_wifi.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "api_client.h"
#include "badge.h"
#include "ui.h"
#include "version.h"

#define OTA_BUFFER_SIZE 16 * 1024 // 16 KB

constexpr static const char *TAG = "api_client";

// The ISRG Root X1 certificate embedded in the binary
extern const uint8_t isrgrootx1_cert[] asm("_binary_isrgrootx1_pem_start");
extern const uint8_t isrgrootx1_cert_end[] asm("_binary_isrgrootx1_pem_end");

esp_err_t ApiClient::httpEventHandler(esp_http_client_event_t *evt) {
    auto client = static_cast<RequestContext *>(evt->user_data);
    switch (evt->event_id) {
        case HTTP_EVENT_ERROR: //
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED: //
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADERS_SENT: //
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER: //
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            client->response_headers.emplace(evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA: //
            ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            if (!esp_http_client_is_chunked_response(evt->client)) {
                if (esp_log_level_get(TAG) >= ESP_LOG_DEBUG) {
                    std::cout.write((char *)evt->data, evt->data_len);
                }
                client->response_buffer.append((char *)evt->data, evt->data_len);
            }
            break;
        case HTTP_EVENT_ON_FINISH: //
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED: //
            ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
        case HTTP_EVENT_REDIRECT: //
            ESP_LOGD(TAG, "HTTP_EVENT_REDIRECT");
            break;
        default: //
            ESP_LOGD(TAG, "Unknown event id: %d", evt->event_id);
            break;
    }
    return ESP_OK;
}

ApiClient::ApiResponse ApiClient::doRequest(const std::string_view endpoint, const std::string_view method,
                                            const std::string_view payload) {
    // Per-request context
    auto *context = new RequestContext();

    // Set up the HTTP client configuration
    esp_http_client_config_t config = {};
    std::string url                 = std::string(API_BASE_URL) + endpoint.data();
    config.url                      = url.c_str();
    config.cert_pem                 = reinterpret_cast<const char *>(isrgrootx1_cert);
    config.cert_len                 = isrgrootx1_cert_end - isrgrootx1_cert;
    config.event_handler            = httpEventHandler;
    config.user_data                = context;
    config.method                   = (method == "POST") ? HTTP_METHOD_POST : HTTP_METHOD_GET;

    // Initialize the HTTP client
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == nullptr) {
        ESP_LOGE(TAG, "Failed to initialize HTTP client");
        return ApiResponse();
    }

    // Set the X-API-Key header
    esp_http_client_set_header(client, "X-API-Key", api_key.c_str());

    // Set the payload if it exists for POST requests
    if (!payload.empty()) {
        esp_http_client_set_header(client, "Content-Type", "application/json");
        esp_http_client_set_post_field(client, payload.data(), payload.length());
    }

    ESP_LOGD(TAG, "HTTP Request: %s %s -- %s", config.method == HTTP_METHOD_POST ? "POST" : "GET", config.url,
             payload.empty() ? "" : payload.data());

    // Perform the HTTP request
    ApiResponse response;
    if (esp_err_t err = esp_http_client_perform(client); err != ESP_OK) {
        ESP_LOGE(TAG, "HTTP request failed: %s", esp_err_to_name(err));
    } else {
        response.status_code = esp_http_client_get_status_code(client);
        response.body        = context->response_buffer;
        response.headers     = context->response_headers;
        ESP_LOGD(TAG, "HTTP Status = %d, content_length = %d", response.status_code,
                 (int)esp_http_client_get_content_length(client));
    }

    // Clean up the HTTP client and request context
    esp_http_client_cleanup(client);
    delete context;

    // // Update the WiFi status if the request failed
    // if (badge_state.wifi_status == WIFI_STATUS_CONNECTED && response.status_code >= 500) {
    //     set_status_wifi_state(WIFI_STATUS_SERVER_ERROR);
    // }

    return response;
}

// ------------------------------------------------------------------------------------------------
// Auth API endpoints
// ------------------------------------------------------------------------------------------------

ApiClient::ApiResponse ApiClient::requestAuthCode() {
    return doRequest("/auth/request_auth_code", "POST");
}

ApiClient::ApiResponse ApiClient::authLevelUpCode() {
    return doRequest("/auth/request_levelup_code", "POST");
}

ApiClient::ApiResponse ApiClient::authStatus(const uint32_t irCode) {
    json payload = {{"ir_code", irCode}};
    return doRequest("/auth/auth_status", "POST", payload.dump());
}

// ------------------------------------------------------------------------------------------------
// Badge API endpoints
// ------------------------------------------------------------------------------------------------

ApiClient::ApiResponse ApiClient::getBadgeData() {
    return doRequest("/badge", "GET");
}

ApiClient::ApiResponse ApiClient::registerBadge(const std::string_view handle) {
    json payload = {{"handle", handle}};
    return doRequest("/badge/register", "POST", payload.dump());
}

ApiClient::ApiResponse ApiClient::getFirmwareVersion() {
    return doRequest("/badge/firmware_version", "GET");
}

api_err_t ApiClient::doFirmwareUpdate() {
    // Set up the HTTP client configuration
    esp_http_client_config_t config = {};
    std::string url                 = std::string(API_BASE_URL) + "/badge/firmware";
    config.url                      = url.c_str();
    config.cert_pem                 = (const char *)isrgrootx1_cert;
    config.cert_len                 = isrgrootx1_cert_end - isrgrootx1_cert;
    config.method                   = HTTP_METHOD_GET;
    config.user_data                = this;
    config.keep_alive_enable        = true;
    config.timeout_ms               = 10000;
    config.buffer_size              = OTA_BUFFER_SIZE;

    // Set up the OTA configuration
    esp_https_ota_config_t ota_config = {};
    ota_config.bulk_flash_erase       = true;
    ota_config.partial_http_download  = true;
    ota_config.max_http_request_size  = OTA_BUFFER_SIZE;
    ota_config.http_config            = &config;
    ota_config.http_client_init_cb    = [](esp_http_client_handle_t client) -> esp_err_t {
        ESP_LOGD(TAG, "Injecting X-API-Key header");
        ApiClient *self = nullptr;
        esp_err_t ret   = ESP_OK;
        ret             = esp_http_client_get_user_data(client, (void **)&self);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to get user data from client");
        } else if (self == nullptr) {
            ESP_LOGE(TAG, "User data is NULL");
        } else if (self->api_key.empty()) {
            ESP_LOGE(TAG, "API key is empty");
        } else if ((ret = esp_http_client_set_header(client, "X-API-Key", self->api_key.c_str())) != ESP_OK) {
            ESP_LOGE(TAG, "Failed to set X-API-Key header");
        } else {
            ESP_LOGD(TAG, "Set X-API-Key header to: %s", self->api_key.c_str());
        }
        return ret;
    };

    // Error var to capture any errors during the OTA process
    esp_err_t err;
    bool checking = false;

    // Get the current power save mode and disable it for the duration OTA process
    wifi_ps_type_t orig_wifi_ps_type;
    esp_wifi_get_ps(&orig_wifi_ps_type);
    esp_wifi_set_ps(WIFI_PS_NONE);

    // Start the OTA process
    esp_https_ota_handle_t ota_handle = nullptr;
    if (esp_https_ota_begin(&ota_config, &ota_handle) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to begin HTTPS OTA");
        return api_err_t::API_FAIL;
    } else {
        checking = true;
        set_ota_status(ota_status_t::OTA_STATUS_CHECKING);
    }

    // Abort helper function for OTA process to clean up and return an error
    auto ota_abort = [&checking, orig_wifi_ps_type, &ota_handle](std::string_view message = "", auto... args) {
        if (!message.empty()) {
            esp_log_write(ESP_LOG_WARN, TAG,
                          (std::string(LOG_COLOR_W "W (%lu) %s: ") + message.data() + LOG_RESET_COLOR + "\n").c_str(),
                          esp_log_timestamp(), TAG, args...);
        }
        esp_wifi_set_ps(orig_wifi_ps_type);
        esp_https_ota_abort(ota_handle);
        set_ota_message(message.empty() ? "Update failed" : message.data());
        set_ota_status(checking ? ota_status_t::OTA_STATUS_CHECK_FAILED : ota_status_t::OTA_STATUS_FAILED);
        checking = false;
        return api_err_t::API_FAIL;
    };

    // Get the OTA image header
    esp_app_desc_t ota_app_desc;
    if ((err = esp_https_ota_get_img_desc(ota_handle, &ota_app_desc)) != ESP_OK) {
        return ota_abort("Failed to get image description: %s", esp_err_to_name(err));
    }

    // Validate the OTA image version against the current firmware version
    const esp_partition_t *running_partition = esp_ota_get_running_partition();
    esp_app_desc_t running_app_desc;
    if (esp_ota_get_partition_description(running_partition, &running_app_desc) != ESP_OK) {
        return ota_abort("Failed to get running partition description");
    } else {
        version_t running_version;
        version_t ota_version;
        version_parse(running_app_desc.version, &running_version);
        version_parse(ota_app_desc.version, &ota_version);
        if (version_compare(&running_version, &ota_version) >= 0) {
            return ota_abort("Firmware is already up to date");
        }
        checking = false;
        set_ota_status(ota_status_t::OTA_STATUS_DOWNLOADING);
    }

    // Main update loop
    while (true) {
        err = esp_https_ota_perform(ota_handle);
        if (err != ESP_ERR_HTTPS_OTA_IN_PROGRESS) {
            break;
        }
        int bytes_read = esp_https_ota_get_image_len_read(ota_handle);
        int total_size = esp_https_ota_get_image_size(ota_handle);
        ESP_LOGD(TAG, "OTA in progress... read: %d/%d bytes [%.2f%%]", bytes_read, total_size,
                 double(bytes_read * 100) / total_size);
        set_ota_progress({bytes_read, total_size});
    }

    if (esp_https_ota_is_complete_data_received(ota_handle) != true) {
        return ota_abort("Failed to receive complete data");
    } else {
        esp_err_t fin_err = esp_https_ota_finish(ota_handle);
        if ((err == ESP_OK) && (fin_err == ESP_OK)) {
            set_ota_message("Upgrade complete... rebooting");
            ESP_LOGI(TAG, "OTA upgrade finished successfully... rebooting");
            vTaskDelay(pdMS_TO_TICKS(1000));
            set_ota_status(ota_status_t::OTA_STATUS_SUCCESS);
            esp_restart();
        } else {
            if (fin_err == ESP_ERR_OTA_VALIDATE_FAILED) {
                return ota_abort("Failed to validate OTA image");
            }
            return ota_abort("Failed to finish OTA: %s", esp_err_to_name(fin_err));
        }
    }

    // Restore the original WiFi power save mode
    esp_wifi_set_ps(orig_wifi_ps_type);

    return api_err_t::API_OK;
}

ApiClient::ApiResponse ApiClient::joinTower(const uint32_t towerIrCode) {
    json payload = {{"tower_ir_code", towerIrCode}};
    return doRequest("/badge/join_tower", "POST", payload.dump());
}

api_err_t ApiClient::leaveTower() {
    if (ApiResponse response = doRequest("/badge/leave_tower", "POST");
        response.status_code >= 300 || response.body.empty() || response.body_json()["status"] != true) {
        return api_err_t::API_FAIL;
    }

    return api_err_t::API_OK;
}

ApiClient::ApiResponse ApiClient::joinBattle() {
    return doRequest("/badge/join_battle", "POST");
}

ApiClient::ApiResponse ApiClient::getTowerStatus(const int towerId) {
    return doRequest(std::format("/badge/tower_status?tower_id={}", towerId), "GET");
}

ApiClient::ApiResponse ApiClient::getTowerStatus(const uint32_t towerIrCode) {
    return doRequest(std::format("/badge/tower_status?ir_code={}", towerIrCode), "GET");
}

ApiClient::ApiResponse ApiClient::getAllTowerStatus() {
    return doRequest("/badge/all_tower_status", "GET");
}

ApiClient::ApiResponse ApiClient::checkIrCodes(const std::vector<uint32_t> &irCodes) {
    json payload = {{"codes", irCodes}};
    return doRequest("/badge/ir_code", "POST", payload.dump());
}

ApiClient::ApiResponse ApiClient::equipMinibadge(const std::string_view slot1, const std::string_view slot2) {
    json payload = {{"slot1", slot1}, {"slot2", slot2}};
    return doRequest("/badge/equip", "POST", payload.dump());
}

ApiClient::ApiResponse ApiClient::requestLevelUp(const int level) {
    json payload = {{"level", level}};
    return doRequest("/badge/levelup", "POST", payload.dump());
}

// ------------------------------------------------------------------------------------------------
// Vending API endpoints
// ------------------------------------------------------------------------------------------------

ApiClient::ApiResponse ApiClient::vendItems() {
    return doRequest("/vend/items", "GET");
}

ApiClient::ApiResponse ApiClient::vendBuyItem(int itemId) {
    json payload = {{"item_id", itemId}};
    return doRequest("/vend/buy", "POST", payload.dump());
}

// ------------------------------------------------------------------------------------------------
// Battle API endpoints
// ------------------------------------------------------------------------------------------------

ApiClient::ApiResponse ApiClient::sendAttack(const int battleId, const int stratagemLength, const int stratagemCount,
                                             const uint32_t attackDurationMs) {
    json payload = {{"battle_id", battleId},
                    {"length", stratagemLength},
                    {"amount", stratagemCount},
                    {"time", (float)attackDurationMs / 1000}};
    return doRequest("/battle/attack", "POST", payload.dump());
}

ApiClient::ApiResponse ApiClient::sendFail(const int battleId) {
    json payload = {{"battle_id", battleId}};
    return doRequest("/battle/fail", "POST", payload.dump());
}

ApiClient::ApiResponse ApiClient::getBattleStatus(const int battleId) {
    return doRequest(std::format("/battle/status/{}", battleId), "GET");
}

ApiClient::ApiResponse ApiClient::getSaviorCode() {
    return doRequest("/battle/savior", "GET");
}

ApiClient::ApiResponse ApiClient::selfSave(const int battleId) {
    json payload = {{"battle_id", battleId}};
    return doRequest("/battle/self_save", "POST", payload.dump());
}

ApiClient::ApiResponse ApiClient::afterActionReport(const int battleId) {
    json payload = {{"battle_id", battleId}};
    return doRequest("/battle/aar", "POST", payload.dump());
}
