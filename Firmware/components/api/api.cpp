#include <algorithm>
#include <concepts>
#include <memory>
#include <ranges>
#include <string>
#include "api.h"
#include "api_client.h"
#include "esp_log.h"

constexpr static const char *TAG = "api";

// Global instance of ApiClient
const static std::unique_ptr<ApiClient> apiClient = std::make_unique<ApiClient>();

// Function prototypes
char *getstr(const json &json, const char *key);
void nullsafe_free(void *ptr);

extern "C" void api_free_result_data(void *data, api_result_type_t type) {
    if (data == nullptr) {
        return;
    }

    switch (type) {
        case api_result_type_t::API_AUTH_CODE_STATUS: {
            auto auth_code_status = (api_auth_code_status_t *)data;
            nullsafe_free(auth_code_status->request_time);
            nullsafe_free(auth_code_status->claimed_by);
            nullsafe_free(auth_code_status->claim_time);
            break;
        }
        case api_result_type_t::API_BADGE_DATA: {
            auto badge_data = (api_badge_data_t *)data;
            nullsafe_free(badge_data->handle);
            nullsafe_free(badge_data->badge_id);
            nullsafe_free(badge_data->community);
            break;
        }
        case api_result_type_t::API_FIRMWARE_DATA: {
            auto firmware_data = (api_firmware_data_t *)data;
            nullsafe_free(firmware_data->setting);
            nullsafe_free(firmware_data->value);
            break;
        }
        case api_result_type_t::API_TOWER_STATUS: {
            auto tower_status = (api_tower_info_t *)data;
            nullsafe_free(tower_status->name);
            nullsafe_free(tower_status->location);
            nullsafe_free(tower_status->boot_time);
            break;
        }
        case api_result_type_t::API_ALL_TOWER_STATUS: {
            auto all_tower_status = (api_all_tower_info_t *)data;
            for (size_t i = 0; i < all_tower_status->count; i++) {
                nullsafe_free(all_tower_status->towers[i].name);
                nullsafe_free(all_tower_status->towers[i].location);
                nullsafe_free(all_tower_status->towers[i].boot_time);
            }
            nullsafe_free(all_tower_status->towers);
            break;
        }
        case api_result_type_t::API_IR_CODE: {
            auto ir_code_result = (api_ir_code_result_t *)data;
            for (size_t i = 0; i < ir_code_result->count; i++) {
                nullsafe_free(ir_code_result->ir_codes[i].name);
                nullsafe_free(ir_code_result->ir_codes[i].response);
            }
            nullsafe_free(ir_code_result->ir_codes);
            break;
        }
        case api_result_type_t::API_EQUIP_MINIBADGE: {
            auto equip_minibadge_data = (api_equip_minibadge_t *)data;
            nullsafe_free(equip_minibadge_data->slot1.slot);
            nullsafe_free(equip_minibadge_data->slot1.name);
            nullsafe_free(equip_minibadge_data->slot1.shortname);
            nullsafe_free(equip_minibadge_data->slot2.slot);
            nullsafe_free(equip_minibadge_data->slot2.name);
            nullsafe_free(equip_minibadge_data->slot2.shortname);
            break;
        }
        case api_result_type_t::API_VEND_ITEMS: {
            auto vend_items_data = (api_vend_items_t *)data;
            for (size_t i = 0; i < vend_items_data->count; i++) {
                nullsafe_free(vend_items_data->items[i].item_name);
                nullsafe_free(vend_items_data->items[i].image_url);
            }
            nullsafe_free(vend_items_data->items);
            break;
        }
        case api_result_type_t::API_BATTLE_STATUS: {
            auto battle_status = (api_battle_status_t *)data;
            for (size_t i = 0; i < battle_status->savior_handle_count; i++) {
                nullsafe_free(battle_status->savior_handle[i]);
            }
            nullsafe_free(battle_status->savior_handle);
            break;
        }
        default: //
            break;
    }

    nullsafe_free(data);
}

extern "C" void api_free_result(api_result_t *result, bool free_data) {
    if (result == nullptr) {
        return;
    }

    nullsafe_free(result->detail);

    if (result->data != nullptr && free_data) {
        api_free_result_data(result->data, result->type);
    }

    nullsafe_free(result);
}

api_result_t *base_result(const ApiClient::ApiResponse &response) {
    // All valid responses should have a body with a JSON object or array
    if (response.body.empty()) {
        return nullptr;
    }
    ESP_LOGD(TAG, "Response Body: %s", response.body.c_str());

    // Check if the response body is valid JSON
    if (!json::accept(response.body)) {
        ESP_LOGE(TAG, "Invalid JSON response: %s", response.body.c_str());
        return nullptr;
    }

    // Parse the response body as JSON
    json response_json = response.body_json();
    if (response_json.is_null() || !response_json.is_object()) {
        ESP_LOGE(TAG, "JSON is null or not an object: %s", response.body.c_str());
        return nullptr;
    }
    ESP_LOGD(TAG, "Response JSON: %s", response_json.dump().c_str());

    // Create a new result struct to return
    auto result = (api_result_t *)malloc(sizeof(api_result_t));
    if (result == nullptr) {
        return nullptr;
    }

    // Populate the basic fields
    result->type   = api_result_type_t::API_BASE;
    result->status = response_json.contains("status") ? (bool)response_json["status"]
                                                      : response.status_code >= 200 && response.status_code < 300;
    result->detail = nullptr;
    result->data   = nullptr;

    // Get the detail field
    if (response_json.contains("detail") && response_json["detail"] != nullptr) {
        if (response_json["detail"].is_array()) {
            result->detail = strdup(response_json["detail"][0]["msg"].get<std::string>().c_str());
        } else if (response_json["detail"].is_string()) {
            result->detail = strdup(response_json["detail"].get<std::string>().c_str());
        }
    }

    return result;
}

char *getstr(const json &json, const char *key) {
    if (json.contains(key) && json[key] != nullptr) {
        return strdup(json[key].get<std::string>().c_str());
    }
    return nullptr;
}

void nullsafe_free(void *ptr) {
    if (ptr != nullptr) {
        free(ptr);
        ptr = nullptr;
    }
}

extern "C" api_result_t *api_request_auth_code() {
    if (apiClient == nullptr) {
        return nullptr;
    }

    auto response = apiClient->requestAuthCode();
    auto result   = base_result(response);
    if (result == nullptr) {
        return nullptr;
    }

    auto result_json = response.body_json()["result"];
    if (result_json.is_null() || !result_json.is_object()) {
        api_free_result(result, true);
        return nullptr;
    }

    result->data = malloc(sizeof(api_ir_code_only_t));
    if (result->data == nullptr) {
        api_free_result(result, true);
        return nullptr;
    }

    // Set the result type
    result->type = api_result_type_t::API_IR_CODE_ONLY;

    // Copy the IR code
    auto ir_code_data  = (api_ir_code_only_t *)result->data;
    ir_code_data->code = result_json["code"];

    return result;
}

extern "C" api_result_t *api_auth_levelup_code() {
    if (apiClient == nullptr) {
        return nullptr;
    }

    auto response = apiClient->authLevelUpCode();
    auto result   = base_result(response);
    if (result == nullptr) {
        return nullptr;
    }

    auto result_json = response.body_json()["result"];
    if (result_json.is_null() || !result_json.is_object()) {
        api_free_result(result, true);
        return nullptr;
    }

    result->data = malloc(sizeof(api_ir_code_only_t));
    if (result->data == nullptr) {
        api_free_result(result, true);
        return nullptr;
    }

    // Set the result type
    result->type = api_result_type_t::API_IR_CODE_ONLY;

    // Copy the IR code
    auto ir_code_data  = (api_ir_code_only_t *)result->data;
    ir_code_data->code = result_json["code"];

    return result;
}

extern "C" api_result_t *api_auth_status(const uint32_t ir_code) {
    if (apiClient == nullptr) {
        return nullptr;
    }

    auto response = apiClient->authStatus(ir_code);
    auto result   = base_result(response);
    if (result == nullptr) {
        return nullptr;
    }

    auto result_json = response.body_json()["result"];
    if (result_json.is_null() || !result_json.is_object()) {
        api_free_result(result, true);
        return nullptr;
    }

    result->data = malloc(sizeof(api_auth_code_status_t));
    if (result->data == nullptr) {
        api_free_result(result, true);
        return nullptr;
    }

    // Set the result type
    result->type = api_result_type_t::API_AUTH_CODE_STATUS;

    // Copy the auth code status data
    auto auth_code_status          = (api_auth_code_status_t *)result->data;
    auth_code_status->code         = result_json["code"];
    auth_code_status->claimed      = result_json["claimed"];
    auth_code_status->request_time = getstr(result_json, "request_time");
    auth_code_status->claimed_by   = getstr(result_json, "claimed_by");
    auth_code_status->claim_time   = getstr(result_json, "claim_time");

    return result;
}

extern "C" api_result_t *api_get_badge_data() {
    if (apiClient == nullptr) {
        return nullptr;
    }

    auto response = apiClient->getBadgeData();

    // Create a new result struct to return
    auto result = base_result(response);
    if (result == nullptr) {
        return nullptr;
    }

    // Parse the response JSON
    auto result_json = response.body_json()["result"];
    if (result_json.is_null() || !result_json.is_object()) {
        api_free_result(result, true);
        return nullptr;
    }

    // Create a new result struct to return in the data field
    result->data = malloc(sizeof(api_badge_data_t));
    if (result->data == nullptr) {
        api_free_result(result, true);
        return nullptr;
    }

    // Set the result type
    result->type = api_result_type_t::API_BADGE_DATA;

    // Copy the badge data
    auto badge_data              = (api_badge_data_t *)result->data;
    badge_data->id               = result_json["id"];
    badge_data->badge_id         = getstr(result_json, "badge_id");
    badge_data->handle           = getstr(result_json, "handle");
    badge_data->xp               = result_json["xp"];
    badge_data->level            = result_json["level"];
    badge_data->enabled          = result_json["enabled"];
    badge_data->badge_team       = result_json["badge_team"];
    badge_data->staff            = result_json["staff"];
    badge_data->blackbadge       = result_json["blackbadge"];
    badge_data->can_level        = result_json["can_level"];
    badge_data->community        = getstr(result_json, "community");
    badge_data->community_levels = result_json["community_levels"].is_null() ? 0 : (int)result_json["community_levels"];
    badge_data->is_savior        = result_json["is_savior"];
    badge_data->coins            = result_json["coins"].is_null() ? 0 : (int)result_json["coins"];
    return result;
}

extern "C" api_result_t *api_register(const char *handle) {
    if (apiClient == nullptr) {
        return nullptr;
    }
    return base_result(apiClient->registerBadge(handle));
}

extern "C" api_result_t *api_get_firmware_version() {
    if (apiClient == nullptr) {
        return nullptr;
    }

    auto response = apiClient->getFirmwareVersion();

    // Create a new result struct to return
    auto result = base_result(response);
    if (result == nullptr) {
        return nullptr;
    }

    // Parse the response JSON
    auto result_json = response.body_json()["result"];
    if (result_json.is_null() || !result_json.is_object()) {
        api_free_result(result, true);
        return nullptr;
    }

    // Create a new result struct to return in the data field
    result->data = malloc(sizeof(api_firmware_data_t));
    if (result->data == nullptr) {
        api_free_result(result, true);
        return nullptr;
    }

    // Set the result type
    result->type = api_result_type_t::API_FIRMWARE_DATA;

    // Copy the firmware data
    auto firmware_data     = (api_firmware_data_t *)result->data;
    firmware_data->setting = getstr(result_json, "setting");
    firmware_data->value   = getstr(result_json, "value");

    return result;
}

extern "C" api_err_t api_do_firmware_update() {
    if (apiClient == nullptr) {
        return api_err_t::API_FAIL;
    }
    return apiClient->doFirmwareUpdate();
}

extern "C" api_result_t *api_join_tower(const uint32_t ir_code) {
    if (apiClient == nullptr) {
        return nullptr;
    }
    return base_result(apiClient->joinTower(ir_code));
}

extern "C" api_err_t api_leave_tower() {
    if (apiClient == nullptr) {
        return api_err_t::API_FAIL;
    }
    return apiClient->leaveTower();
}

extern "C" api_result_t *api_join_battle() {
    if (apiClient == nullptr) {
        return nullptr;
    }

    auto response = apiClient->joinBattle();

    // Create a new result struct to return
    auto result = base_result(response);
    if (result == nullptr) {
        return nullptr;
    }

    // Parse the response JSON
    auto result_json = response.body_json()["result"];
    if (result_json.is_null() || !result_json.is_object()) {
        api_free_result(result, true);
        return nullptr;
    }

    // Create a new result struct to return in the data field
    result->data = malloc(sizeof(api_join_battle_t));
    if (result->data == nullptr) {
        api_free_result(result, true);
        return nullptr;
    }

    // Set the result type
    result->type = api_result_type_t::API_JOIN_BATTLE;

    // Copy the battle ID
    auto join_battle_data       = (api_join_battle_t *)result->data;
    join_battle_data->battle_id = result_json["battle_id"];

    return result;
}

template <typename T> static api_result_t *api_get_tower_status_impl(const T &id) {
    if (apiClient == nullptr) {
        return nullptr;
    }

    auto response = apiClient->getTowerStatus(id);

    // Create a new result struct to return
    auto result = base_result(response);
    if (result == nullptr) {
        return nullptr;
    }

    // Parse the response JSON
    auto result_json = response.body_json()["result"];
    if (result_json.is_null() || !result_json.is_object()) {
        api_free_result(result, true);
        return nullptr;
    }

    // Create a new result struct to return in the data field
    result->data = malloc(sizeof(api_tower_info_t));
    if (result->data == nullptr) {
        api_free_result(result, true);
        return nullptr;
    }

    // Set the result type
    result->type = api_result_type_t::API_TOWER_STATUS;

    // Copy the tower status data
    auto tower_status                  = (api_tower_info_t *)result->data;
    tower_status->id                   = result_json["id"];
    tower_status->name                 = getstr(result_json, "name");
    tower_status->location             = getstr(result_json, "location");
    tower_status->level                = result_json["level"];
    tower_status->health               = result_json["health"];
    tower_status->max_health           = result_json["max_health"];
    tower_status->boot_time            = getstr(result_json, "boot_time");
    tower_status->ir_code              = result_json["ir_code"];
    tower_status->enabled              = result_json["enabled"];
    tower_status->status               = get_tower_status(result_json["status"].template get<std::string>().c_str());
    tower_status->players_in_range     = result_json["players_in_range"];
    tower_status->players_in_battle    = result_json["players_in_battle"];
    tower_status->players_joined_tower = result_json["players_joined_tower"];
    tower_status->players_disconnected = result_json["players_disconnected"];

    return result;
}

extern "C" api_result_t *api_get_tower_status_by_id(int tower_id) {
    return api_get_tower_status_impl(tower_id);
}

extern "C" api_result_t *api_get_tower_status_by_ir_code(const uint32_t ir_code) {
    return api_get_tower_status_impl(ir_code);
}

extern "C" api_result_t *api_get_all_tower_status() {
    if (apiClient == nullptr) {
        return nullptr;
    }

    auto response = apiClient->getAllTowerStatus();

    // Create a new result struct to return
    auto result = base_result(response);
    if (result == nullptr) {
        return nullptr;
    }

    // Parse the response JSON
    auto result_json = response.body_json()["result"];
    if (result_json.is_null() || !result_json.is_array()) {
        api_free_result(result, true);
        return nullptr;
    }

    // Create a new result struct to return in the data field
    result->data = malloc(sizeof(api_all_tower_info_t));
    if (result->data == nullptr) {
        api_free_result(result, true);
        return nullptr;
    }

    // Set the result type
    result->type = api_result_type_t::API_ALL_TOWER_STATUS;

    // Copy the tower status data
    auto all_tower_status   = (api_all_tower_info_t *)result->data;
    all_tower_status->count = result_json.size();
    ESP_LOGD(TAG, "Tower count: %d", all_tower_status->count);
    all_tower_status->towers = (api_tower_info_t *)malloc(sizeof(api_tower_info_t) * all_tower_status->count);
    if (all_tower_status->towers == nullptr) {
        api_free_result(result, true);
        return nullptr;
    }
    for (size_t i = 0; i < all_tower_status->count; i++) {
        auto tower_json                                  = result_json[i];
        all_tower_status->towers[i].id                   = tower_json["id"];
        all_tower_status->towers[i].name                 = getstr(tower_json, "name");
        all_tower_status->towers[i].location             = getstr(tower_json, "location");
        all_tower_status->towers[i].level                = tower_json["level"];
        all_tower_status->towers[i].health               = tower_json["health"];
        all_tower_status->towers[i].max_health           = tower_json["max_health"];
        all_tower_status->towers[i].boot_time            = getstr(tower_json, "boot_time");
        all_tower_status->towers[i].ir_code              = tower_json["ir_code"];
        all_tower_status->towers[i].enabled              = tower_json["enabled"];
        all_tower_status->towers[i].status               = get_tower_status(tower_json["status"].get<std::string>().c_str());
        all_tower_status->towers[i].players_in_range     = tower_json["players_in_range"];
        all_tower_status->towers[i].players_in_battle    = tower_json["players_in_battle"];
        all_tower_status->towers[i].players_joined_tower = tower_json["players_joined_tower"];
        all_tower_status->towers[i].players_disconnected = tower_json["players_disconnected"];
    }

    return result;
}

extern "C" api_result_t *api_check_ir_codes(const uint32_t *ir_codes, size_t num_codes) {
    if (apiClient == nullptr) {
        return nullptr;
    }

    std::vector<uint32_t> codes;
    for (size_t i = 0; i < num_codes; i++) {
        codes.emplace_back(ir_codes[i]);
    }
    auto response = apiClient->checkIrCodes(codes);

    // Create a new result struct to return
    auto result = base_result(response);
    if (result == nullptr) {
        return nullptr;
    }

    // Parse the response JSON
    auto result_json = response.body_json()["result"];
    if (result_json.is_null() || !result_json.is_array()) {
        api_free_result(result, true);
        return nullptr;
    }

    // Create a new result struct to return in the data field
    result->data = malloc(sizeof(api_ir_code_result_t));
    if (result->data == nullptr) {
        api_free_result(result, true);
        return nullptr;
    }

    // Set the result type
    result->type = api_result_type_t::API_IR_CODE;

    // Copy the IR codes
    auto ir_code_result      = (api_ir_code_result_t *)result->data;
    ir_code_result->count    = result_json.size();
    ir_code_result->ir_codes = (api_ir_code_t *)malloc(sizeof(api_ir_code_t) * ir_code_result->count);
    if (ir_code_result->ir_codes == nullptr) {
        api_free_result(result, true);
        return nullptr;
    }
    for (size_t i = 0; i < ir_code_result->count; i++) {
        bool is_valid                        = result_json[i]["is_valid"];
        ir_code_result->ir_codes[i].code     = result_json[i]["code"];
        ir_code_result->ir_codes[i].is_valid = is_valid;
        if (is_valid) {
            ir_code_result->ir_codes[i].type     = get_ir_code_type(result_json[i]["type"].get<std::string>().c_str());
            ir_code_result->ir_codes[i].name     = getstr(result_json[i], "name");
            ir_code_result->ir_codes[i].response = getstr(result_json[i], "response");
            ir_code_result->ir_codes[i].tower_id = result_json[i].contains("tower_id") ? (int)result_json[i]["tower_id"] : 0;
        } else {
            ir_code_result->ir_codes[i].type     = IR_CODE_TYPE_UNKNOWN;
            ir_code_result->ir_codes[i].name     = nullptr;
            ir_code_result->ir_codes[i].response = nullptr;
            ir_code_result->ir_codes[i].tower_id = 0;
        }
    }

    return result;
}

extern "C" api_result_t *api_equip_minibadge(const char *slot1, const char *slot2) {
    if (apiClient == nullptr) {
        return nullptr;
    }

    auto response = apiClient->equipMinibadge(slot1, slot2);

    // Create a new result struct to return
    auto result = base_result(response);
    if (result == nullptr) {
        return nullptr;
    }

    // Parse the response JSON
    auto result_json = response.body_json()["result"];
    if (result_json.is_null() || !result_json.is_object()) {
        api_free_result(result, true);
        return nullptr;
    }

    // Create a new result struct to return in the data field
    result->data = malloc(sizeof(api_equip_minibadge_t));
    if (result->data == nullptr) {
        api_free_result(result, true);
        return nullptr;
    }

    // Set the result type
    result->type = api_result_type_t::API_EQUIP_MINIBADGE;

    // Copy the equip minibadge data
    auto equip_minibadge_data              = (api_equip_minibadge_t *)result->data;
    const char *slot_names[]               = {"slot1", "slot2"};
    api_minibadge_slot_info_t *slot_info[] = {&equip_minibadge_data->slot1, &equip_minibadge_data->slot2};
    for (size_t i = 0; i < sizeof(slot_info) / sizeof(slot_info[0]); i++) {
        auto slot_json = result_json[slot_names[i]];
        if (slot_json.is_null() || !slot_json.is_object()) {
            slot_info[i]->slot       = strdup(slot_names[i]);
            slot_info[i]->name       = nullptr;
            slot_info[i]->shortname  = nullptr;
            slot_info[i]->buff_type  = MINIBADGE_BUFF_TYPE_NONE;
            slot_info[i]->buff_value = 0.0f;
            slot_info[i]->valid      = false;
            slot_info[i]->rewards    = nullptr;
        } else {
            slot_info[i]->slot       = strdup(slot_names[i]);
            slot_info[i]->name       = strdup(slot_json["name"].get<std::string>().c_str());
            slot_info[i]->shortname  = strdup(slot_json["shortname"].get<std::string>().c_str());
            slot_info[i]->buff_type  = get_minibadge_buff_type(slot_json["buff_type"].get<std::string>().c_str());
            slot_info[i]->buff_value = slot_json["buff_value"];
            slot_info[i]->valid      = slot_json["valid"];
            slot_info[i]->rewards    = slot_json.contains("rewards") // This seems to only maybe exist
                                           ? strdup(slot_json["rewards"].get<std::string>().c_str())
                                           : nullptr;
        }
    }

    return result;
}

extern "C" api_result_t *api_request_level_up(const int level) {
    if (apiClient == nullptr) {
        return nullptr;
    }

    auto response = apiClient->requestLevelUp(level);

    // Create a new result struct to return
    auto result = base_result(response);
    if (result == nullptr) {
        return nullptr;
    }

    // Parse the response JSON
    auto result_json = response.body_json()["result"];
    if (result_json.is_null() || !result_json.is_object()) {
        api_free_result(result, true);
        return nullptr;
    }

    // Create a new result struct to return in the data field
    result->data = malloc(sizeof(api_ir_code_only_t));
    if (result->data == nullptr) {
        api_free_result(result, true);
        return nullptr;
    }

    // Set the result type
    result->type = api_result_type_t::API_IR_CODE_ONLY;

    // Copy the level up code
    auto request_level_up_data  = (api_ir_code_only_t *)result->data;
    request_level_up_data->code = result_json["code"];

    return result;
}

extern "C" api_result_t *api_vend_get_items() {
    if (apiClient == nullptr) {
        return nullptr;
    }

    auto response = apiClient->vendItems();

    // Create a new result struct to return
    auto result = base_result(response);
    if (result == nullptr) {
        return nullptr;
    }

    // Parse the response JSON
    auto result_json = response.body_json()["result"];
    if (result_json.is_null() || !result_json.is_array()) {
        api_free_result(result, true);
        return nullptr;
    }

    // Create a new result struct to return in the data field
    result->data = malloc(sizeof(api_vend_items_t));
    if (result->data == nullptr) {
        api_free_result(result, true);
        return nullptr;
    }

    // Set the result type
    result->type = api_result_type_t::API_VEND_ITEMS;

    // Copy the vend items data
    auto vend_items_data   = (api_vend_items_t *)result->data;
    vend_items_data->count = result_json.size();
    vend_items_data->items = (api_vend_item_t *)malloc(sizeof(api_vend_item_t) * vend_items_data->count);
    if (vend_items_data->items == nullptr) {
        api_free_result(result, true);
        return nullptr;
    }
    for (size_t i = 0; i < vend_items_data->count; i++) {
        auto item_json                            = result_json[i];
        vend_items_data->items[i].item_id         = item_json["item_id"];
        vend_items_data->items[i].item_name       = getstr(item_json, "item_name");
        vend_items_data->items[i].item_price      = item_json["item_price"];
        vend_items_data->items[i].available_stock = item_json["available_stock"];
        vend_items_data->items[i].purchased       = item_json["purchased"];
        vend_items_data->items[i].sold_out        = item_json["sold_out"];
        vend_items_data->items[i].image_url       = getstr(item_json, "image_url");
        vend_items_data->items[i].version         = item_json["version"];
    }

    return result;
}

extern "C" api_result_t *api_vend_buy_item(int item_id) {
    if (apiClient == nullptr) {
        return nullptr;
    }

    auto response = apiClient->vendBuyItem(item_id);
    return base_result(response);
}

extern "C" api_result_t *api_send_attack(int battle_id, int stratagem_length, int stratagem_count, uint32_t attack_duration) {
    if (apiClient == nullptr) {
        return nullptr;
    }

    auto response = apiClient->sendAttack(battle_id, stratagem_length, stratagem_count, attack_duration);

    // Create a new result struct to return
    auto result = base_result(response);
    if (result == nullptr) {
        return nullptr;
    }

    // Parse the response JSON
    auto result_json = response.body_json()["result"];
    if (result_json.is_null() || !result_json.is_object()) {
        api_free_result(result, true);
        return nullptr;
    }

    // Create a new result struct to return in the data field
    result->data = malloc(sizeof(api_ir_code_only_t));
    if (result->data == nullptr) {
        api_free_result(result, true);
        return nullptr;
    }

    // Set the result type
    result->type = api_result_type_t::API_SEND_ATTACK;

    // Copy the attack data
    auto send_attack_data     = (api_send_attack_t *)result->data;
    send_attack_data->percent = result_json["percent"];

    return result;
}

static bool api_make_battle_status_result(ApiClient::ApiResponse &response, api_result_t *result) {
    if (result == nullptr) {
        return false;
    }

    // Parse the response JSON
    auto result_json = response.body_json()["result"];
    if (result_json.is_null() || !result_json.is_object()) {
        api_free_result(result, true);
        return false;
    }

    // Create a new result struct to return in the data field
    result->data = malloc(sizeof(api_battle_status_t));
    if (result->data == nullptr) {
        api_free_result(result, true);
        return false;
    }

    // Set the result type
    result->type = api_result_type_t::API_BATTLE_STATUS;

    // Copy the battle status data
    auto battle_status              = (api_battle_status_t *)result->data;
    battle_status->battle_active    = result_json["battle_active"];
    battle_status->player_level     = result_json["player_level"].is_null() ? 0 : (int)result_json["player_level"];
    battle_status->player_hp        = result_json["player_hp"].is_null() ? 0 : (int)result_json["player_hp"];
    battle_status->player_status    = get_player_status(result_json["player_status"].get<std::string>().c_str());
    battle_status->is_savior        = result_json["is_savior"];
    battle_status->tower_ir_code    = result_json["tower_ir_code"].is_null() ? 0 : (uint32_t)result_json["tower_ir_code"];
    battle_status->tower_level      = result_json["tower_level"].is_null() ? 0 : (int)result_json["tower_level"];
    battle_status->tower_health     = result_json["tower_health"].is_null() ? 0 : (int)result_json["tower_health"];
    battle_status->tower_max_health = result_json["tower_max_health"].is_null() ? 0 : (int)result_json["tower_max_health"];
    battle_status->players_in_range = result_json["players_in_range"].is_null() ? 0 : (int)result_json["players_in_range"];
    battle_status->players_joined_tower =
        result_json["players_joined_tower"].is_null() ? 0 : (int)result_json["players_joined_tower"];
    battle_status->players_in_battle = result_json["players_in_battle"].is_null() ? 0 : (int)result_json["players_in_battle"];
    battle_status->strategem_min     = result_json["strategem_min"].is_null() ? 0 : (int)result_json["strategem_min"];
    battle_status->strategem_max     = result_json["strategem_max"].is_null() ? 0 : (int)result_json["strategem_max"];
    battle_status->stratagem_amount  = result_json["strategem_amount"].is_null() ? 0 : (int)result_json["strategem_amount"];
    battle_status->players_disconnected =
        result_json["players_disconnected"].is_null() ? 0 : (int)result_json["players_disconnected"];
    if (result_json["savior_handle"].is_array()) {
        battle_status->savior_handle_count = result_json["savior_handle"].size();
        battle_status->savior_handle       = (char **)malloc(sizeof(char *) * battle_status->savior_handle_count);
        for (size_t i = 0; i < battle_status->savior_handle_count; i++) {
            battle_status->savior_handle[i] = strdup(result_json["savior_handle"][i].get<std::string>().c_str());
        }
    }

    return true;
}

extern "C" api_result_t *api_send_attack_fail(int battle_id) {
    if (apiClient == nullptr) {
        return nullptr;
    }

    auto response = apiClient->sendFail(battle_id);

    // Create a new result struct to return
    auto result = base_result(response);
    if (result == nullptr) {
        return nullptr;
    }

    if (!api_make_battle_status_result(response, result)) {
        // Already freed by api_make_battle_status_result
        result = nullptr;
    }

    return result;
}

extern "C" api_result_t *api_get_battle_status(int battle_id) {
    if (apiClient == nullptr) {
        return nullptr;
    }

    auto response = apiClient->getBattleStatus(battle_id);

    // Create a new result struct to return
    auto result = base_result(response);
    if (result == nullptr) {
        return nullptr;
    }

    if (!api_make_battle_status_result(response, result)) {
        // Already freed by api_make_battle_status_result
        result = nullptr;
    }

    return result;
}

extern "C" api_result_t *api_get_savior_code() {
    if (apiClient == nullptr) {
        return nullptr;
    }

    auto response = apiClient->getSaviorCode();

    // Create a new result struct to return
    auto result = base_result(response);
    if (result == nullptr) {
        return nullptr;
    }

    // Parse the response JSON
    auto result_json = response.body_json()["result"];
    if (result_json.is_null() || !result_json.is_object()) {
        api_free_result(result, true);
        return nullptr;
    }

    // Create a new result struct to return in the data field
    result->data = malloc(sizeof(api_ir_code_only_t));
    if (result->data == nullptr) {
        api_free_result(result, true);
        return nullptr;
    }

    // Set the result type
    result->type = api_result_type_t::API_IR_CODE_ONLY;

    // Copy the savior code
    auto savior_code_data  = (api_ir_code_only_t *)result->data;
    savior_code_data->code = result_json["ir_code"];

    return result;
}

extern "C" api_result_t *api_self_save(int battle_id) {
    if (apiClient == nullptr) {
        return nullptr;
    }

    auto response = apiClient->selfSave(battle_id);

    // Create a new result struct to return
    auto result = base_result(response);
    if (result == nullptr) {
        return nullptr;
    }

    if (!api_make_battle_status_result(response, result)) {
        // Already freed by api_make_battle_status_result
        result = nullptr;
    }

    return result;
}

extern "C" api_result_t *api_after_action_report(int battle_id) {
    if (apiClient == nullptr) {
        return nullptr;
    }

    auto response = apiClient->afterActionReport(battle_id);

    // Create a new result struct to return
    auto result = base_result(response);
    if (result == nullptr) {
        return nullptr;
    }

    // Parse the response JSON
    auto result_json = response.body_json()["result"];
    if (result_json.is_null() || !result_json.is_object()) {
        api_free_result(result, true);
        return nullptr;
    }

    // Create a new result struct to return in the data field
    result->data = malloc(sizeof(api_after_action_report_t));
    if (result->data == nullptr) {
        api_free_result(result, true);
        return nullptr;
    }

    // Set the result type
    result->type = api_result_type_t::API_AFTER_ACTION_REPORT;

    // Copy the after action report data
    auto aar_data             = (api_after_action_report_t *)result->data;
    aar_data->total_attacks   = result_json["total_attacks"];
    aar_data->total_damage    = result_json["total_damage"];
    aar_data->xp_earned       = result_json["xp_earned"];
    aar_data->avg_performance = result_json["avg_performance"];
    aar_data->failures        = result_json["failures"];
    aar_data->coins_earned    = result_json["coins_earned"];

    return result;
}
