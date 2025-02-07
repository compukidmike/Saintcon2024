#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "lvgl.h"

#include "minibadge.h"

// Modes for the attack component
typedef enum {
    ATTACK_MODE_TOWER,
    ATTACK_MODE_SAVING_THROW,
    ATTACK_MODE_PVP,
} attack_mode_t;

// The minibadge power-ups - these map 1:1 with the buffs in the minibadge but it's useful to use this terminology here for now
typedef enum {
    POWER_UP_NONE,            // No power-up
    POWER_UP_DPAD,            // Physical D-pad
    POWER_UP_LOOT,            // Loot: More money on each tower encounter
    POWER_UP_STRAT_SIMPLIFY,  // Stratagem simplify: TBD fewer arrows in the stratagem
    POWER_UP_SPELLCHECK,      // Spellcheck: Can miss more arrows without losing
    POWER_UP_QUAD_DAMAGE,     // Quad damage: Damage multiplier decreases at a slower rate
    POWER_UP_GLASS_CANNON,    // Glass cannon: Attacks deal more damage, but _any_ error disconnects you from the battle
    POWER_UP_SAVING_THROW,    // Saving throw: If you are disconnected form too many failures you can save yourself with a super
                              // difficult arrow sequence
    POWER_UP_REVERSE_SHELL,   // Backup reverse shell: You automatically reconnect to the battle because you had a "reverse shell"
    POWER_UP_HYDRA,           // Hydra: If disconnected you can reconnect other players for a short period of time
    POWER_UP_ZODIAKS_SANDALS, // Zodiak's sandals: Number of stratagems you have to do for a successful attack decreases
    POWER_UP_GREED,           // Greed: Same reward as the highest attacker if you are in the top 50% of attackers

    // Max number of power-ups
    POWER_UP_COUNT,
} minibadge_power_up_t;

typedef struct {
    minibadge_power_up_t buff;
    int uses_per_battle;
} battle_powerup_info_t;

// Badge-local power-up info we need to keep track of
extern battle_powerup_info_t battle_powerup_info[];

/**
 * @brief Callback to report current state of the attack while the user is attacking
 *
 * @param battle_id The battle_id
 * @param stratagem_index The index of the stratagem
 * @param stratagem_length The length of the stratagem
 * @param score The current number of correct stratagems entered
 * @param failures The current number of failures (including the current one if it was incorrect)
 * @param time_elapsed The time elapsed since the attack started
 *
 * @return true if the attack should continue, false if the attack should end (e.g. player disconnected)
 */
typedef bool (*stratagem_callback_t)(uint32_t battle_id, uint16_t stratagem_index, uint16_t stratagem_length, uint16_t score,
                                     uint16_t failures, int32_t time_elapsed);

/**
 * @brief Callback type to report attack results
 *
 * @param battle_id The battle_id
 * @param success Whether the attack was successful
 * @param stratagem_length The length of the stratagem codes
 * @param stratagem_count The number of stratagem codes the player has to do for a successful attack
 * @param score The final score of the player for this attack
 * @param attack_duration The duration of the attack in milliseconds
 */
typedef void (*attack_result_callback_t)(uint32_t battle_id, bool success, uint16_t stratagem_length, uint16_t stratagem_count,
                                         uint16_t score, int32_t attack_duration);

/**
 * @brief Exit callback for the attack component... will be called during destruction of the component
 *
 * @param battle_id The battle_id
 * @param completed Whether the attack was completed or the user clicked exit/back
 */
typedef void (*attack_exit_callback_t)(uint32_t battle_id, bool completed);

// Struct for the attack component configuration
typedef struct {
    uint32_t battle_id;       // The battle_id from the badge/join_battle response if mode is Tower, otherwise 0
    attack_mode_t mode;       // The mode of the attack
    uint16_t stratagem_min;   // The minimum number of arrows in the stratagem codes
    uint16_t stratagem_max;   // The maximum number of arrows in the stratagem codes
    uint16_t stratagem_count; // The number of stratagem codes the player has to do for a successful attack
    int16_t tower_health;     // The health of the tower (0-100)
    int16_t player_health;    // The health of the player (0-100)
    minibadge_power_up_t power_ups[MINIBADGE_SLOT_COUNT]; // The power-ups the player has enabled
    stratagem_callback_t stratagem_callback;  // A callback to report the current state of the attack after each stratagem
    attack_result_callback_t result_callback; // A callback to report the battle results
    attack_exit_callback_t exit_callback;     // A callback to report the exit/cleanup of the attack UI
} attack_config_t;

/**
 * @brief Create an attack component to run an attack
 *
 * @param config The attack configuration
 * @return The attack object
 */
lv_obj_t *attack_create(const attack_config_t *config);

/**
 * @brief Update the attack component with the current state of the tower, player, etc.
 *
 * @param attack_obj The attack object
 * @param tower_health The health of the tower (0-100)
 * @param player_health The health of the player (0-100)
 * @param players_in_battle The number of players in the battle
 * @param players_disconnected The number of players that have disconnected
 */
void attack_update_status(lv_obj_t *attack_obj, int16_t tower_health, int16_t player_health, uint16_t players_in_battle,
                          uint16_t players_disconnected);

#ifdef __cplusplus
}
#endif
