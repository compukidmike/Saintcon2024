#pragma once

#include <format>
#include <string>
#include <map>
#include "esp_mac.h"
#include "esp_http_client.h"

#include "api.h"

#define JSON_NOEXCEPTION

#include "nlohmann/json.hpp"

using json = nlohmann::json;

class ApiClient {
  public:
    struct ApiResponse {
        std::string body;
        int status_code = -1;
        std::map<std::string, std::string, std::less<>> headers;
        json body_json() const {
            return json::parse(body);
        }
    };

    ApiClient() {
        // API key comes from the MAC address
        uint8_t mac[6] = {0};
        esp_read_mac(mac, ESP_MAC_WIFI_STA);
        api_key = std::format("{:02X}{:02X}{:02X}{:02X}{:02X}{:02X}", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }

    // Auth API endpoints
    ApiResponse requestAuthCode();
    ApiResponse authLevelUpCode();
    ApiResponse authStatus(const uint32_t irCode);

    // Badge API endpoints
    ApiResponse getBadgeData();
    ApiResponse registerBadge(const std::string_view handle);
    ApiResponse getFirmwareVersion();
    api_err_t doFirmwareUpdate();
    ApiResponse joinTower(const uint32_t towerIrCode);
    api_err_t leaveTower();
    ApiResponse joinBattle();
    ApiResponse getTowerStatus(const int towerId);
    ApiResponse getTowerStatus(const uint32_t towerIrCode);
    ApiResponse getAllTowerStatus();
    ApiResponse checkIrCodes(const std::vector<uint32_t> &irCodes);
    ApiResponse equipMinibadge(const std::string_view slot1, const std::string_view slot2);
    ApiResponse requestLevelUp(const int level);

    // Vending API endpoints
    ApiResponse vendItems();
    ApiResponse vendBuyItem(int itemId);
    /** ... skipping machine vending APIs ...
     *
     *  GET /vend/machine/{machine_id}/work
     *  GET /vend/machine/{machine_id}/work_complete
     *  GET /vend/machine/{machine_id}/request_ir_code
     */

    // Tower APIs

    // Battle API endpoints
    ApiResponse sendAttack(const int battleId, const int stratagemLength, const int stratagemCount,
                           const uint32_t attackDurationMs);
    ApiResponse sendFail(const int battleId);
    ApiResponse getBattleStatus(const int battleId);
    ApiResponse getSaviorCode();
    ApiResponse selfSave(const int battleId);
    ApiResponse afterActionReport(const int battleId);

    // PvP API endpoints - these are preliminary and not fully implemented
    // api_err_t pvpStart();
    // api_err_t pvpJoin();
    // api_err_t pvpStatus();
    // api_err_t pvpSubmit();

  private:
    struct RequestContext {
        std::string response_buffer;
        std::map<std::string, std::string, std::less<>> response_headers;
    };

    ApiResponse doRequest(const std::string_view endpoint, const std::string_view method, const std::string_view payload = "");

    std::string api_key;
    static esp_err_t httpEventHandler(esp_http_client_event_t *evt);
};
