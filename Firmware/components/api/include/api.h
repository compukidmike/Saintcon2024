#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "types.h"

#define API_BASE_URL "https://sc24.redactd.net"

/**
 * @brief API result data free function to free allocated memory for result data structs
 *
 * @param[in] data The result data struct to free
 * @param[in] type The type of the result data struct
 */
void api_free_result_data(void *data, api_result_type_t type);

/**
 * @brief API result free function to free allocated memory for result structs
 *
 * @param[in] result The result struct to free
 * @param[in] free_data Whether to free the result data as well
 */
void api_free_result(api_result_t *result, bool free_data);

/**
 * @brief Request an auth code
 *
 * @return A result struct containing the auth IR code
 */
api_result_t *api_request_auth_code();

/**
 * @brief Request a level up code
 *
 * @return A result struct containing the level up IR code
 */
api_result_t *api_auth_levelup_code();

/**
 * @brief Get the status for a given auth IR code
 *
 * @param[in] ir_code The IR code to get the auth status for
 *
 * @return A result struct containing the auth status
 */
api_result_t *api_auth_status(const uint32_t ir_code);

/**
 * @brief Retrieve badge data
 *
 * @return A result struct containing the badge data
 */
api_result_t *api_get_badge_data();

/**
 * @brief Register the badge for a given handle
 *
 * @param[in] handle The handle to register
 *
 * @return A result struct ontaining the registration status
 */
api_result_t *api_register(const char *handle);

/**
 * @brief Retrieve firmware version
 *
 * @return A result struct containing the latest firmware version
 */
api_result_t *api_get_firmware_version();

/**
 * @brief Retrieve firmware binary
 * TODO: This will need a lot more thought as to how to handle this
 *
 * @return API_OK if the request was successful, API_FAIL otherwise
 */
api_err_t api_do_firmware_update();

/**
 * @brief Join a tower
 *
 * @param[in] tower_ir_code The IR code for the tower to join
 *
 * @return A result struct containing the tower status
 */
api_result_t *api_join_tower(const uint32_t tower_ir_code);

/**
 * @brief Leave the currently joined tower
 *
 * @return API_OK if the request was successful, API_FAIL otherwise
 */
api_err_t api_leave_tower();

/**
 * @brief Join a battle
 *
 * @return A result struct containing the battle ID
 */
api_result_t *api_join_battle();

/**
 * @brief Get the tower status for a given tower
 *
 * @param[in] tower_id The ID of the tower to get the status for
 *
 * @return A result struct containing the tower status
 */
api_result_t *api_get_tower_status_by_id(int tower_id);

/**
 * @brief Get the tower status for a given IR code
 *
 * @param[in] ir_code The IR code of the tower to get the status for
 *
 * @return A result struct containing the tower status
 */
api_result_t *api_get_tower_status_by_ir_code(const uint32_t ir_code);

/**
 * @brief Get the status of all towers
 *
 * @return A result struct containing the status of all towers
 */
api_result_t *api_get_all_tower_status();

/**
 * @brief Check IR codes.
 *
 * @param[in] ir_codes A list of IR codes to check
 * @param[in] num_codes The number of IR codes in the list
 *
 * @return A result struct containing the IR codes that were found
 */
api_result_t *api_check_ir_codes(const uint32_t *ir_codes, size_t num_codes);

/**
 * @brief Equip a minibadge
 *
 * @param[in] slot1 The minibadge to equip in slot 1
 * @param[in] slot2 The minibadge to equip in slot 2
 *
 * @return A result struct containing the minibadge slot info
 */
api_result_t *api_equip_minibadge(const char *slot1, const char *slot2);

/**
 * @brief Request a level up
 *
 * @param[in] level The level to request a level up code for
 *
 * @return A result struct containing the level up code
 */
api_result_t *api_request_level_up(const int level);

/**
 * @brief Retrieve the list of items available to purchase
 *
 * @return A result struct containing the list of items
 */
api_result_t *api_vend_get_items();

/**
 * @brief Purchase an item
 *
 * @param[in] item_id The ID of the item to purchase
 *
 * @return A result struct containing the purchase result
 */
api_result_t *api_vend_buy_item(int item_id);

/**
 * @brief Send an attack
 *
 * @param[in] battle_id The ID of the battle to send the attack for
 * @param[in] stratagem_length The length of the stratagem
 * @param[in] stratagem_count The number of stratagems to send
 * @param[in] attack_duration The duration of the attack in milliseconds
 *
 * @return A result struct containing the speed/accuracy percent
 */
api_result_t *api_send_attack(int battle_id, int stratagem_length, int stratagem_count, uint32_t attack_duration);

/**
 * @brief Send an attack failure
 *
 * @param[in] battle_id The ID of the battle to send the attack failure for
 *
 * @return A result struct containing the battle status
 */
api_result_t *api_send_attack_fail(int battle_id);

/**
 * @brief Get the battle status
 *
 * @param[in] battle_id The ID of the battle to get the status for
 *
 * @return A result struct containing the battle status
 */
api_result_t *api_get_battle_status(int battle_id);

/**
 * @brief Get the savior code if you are a savior
 *
 * @return A result struct containing the savior code
 */
api_result_t *api_get_savior_code();

/**
 * @brief Self save from a battle when the "Backup Reverse Shell" or "Saving Throw" power-ups are active
 *
 * @param[in] battle_id The ID of the battle to self save from
 *
 * @return A result struct containing the battle status
 */
api_result_t *api_self_save(int battle_id);

/**
 * @brief Get the after action report
 *
 * @param[in] battle_id The ID of the battle to get the after action report for
 *
 * @return A result struct containing the after action report
 */
api_result_t *api_after_action_report(int battle_id);

#ifdef __cplusplus
}
#endif