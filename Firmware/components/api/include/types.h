#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#ifdef __cplusplus
enum class api_err_t {
    API_OK   = 0, // API call was successful
    API_FAIL = -1 // API call failed - generic error
};
#else
typedef enum {
    API_OK   = 0, // API call was successful
    API_FAIL = -1 // API call failed - generic error
} api_err_t;
#endif

// -----------------------------------------------------------------------------
// API response data
// -----------------------------------------------------------------------------

typedef enum {
    API_BASE,         // Base API endpoint that just has an empty result field
    API_IR_CODE_ONLY, // IR code only result - a number of endpoints only return an IR code
    API_AUTH_CODE_STATUS,
    API_BADGE_DATA,
    API_FIRMWARE_DATA,
    API_JOIN_BATTLE,
    API_TOWER_STATUS,
    API_ALL_TOWER_STATUS,
    API_IR_CODE,
    API_EQUIP_MINIBADGE,
    API_VEND_ITEMS,
    API_SEND_ATTACK,
    API_BATTLE_STATUS,
    API_AFTER_ACTION_REPORT,
} api_result_type_t;

typedef struct {
    api_result_type_t type;
    bool status;
    char *detail;
    void *data;
} api_result_t;

/**
 * @brief IR code response result from endpoints that return just an IR code
 */
typedef struct {
    uint32_t code; // An IR code that the badge can send
} api_ir_code_only_t;

/**
 * @brief Auth code status result for the /auth/auth_status endpoint
 */
typedef struct {
    uint32_t code;      // The auth IR code
    bool claimed;       // Whether the auth code has been claimed
    char *request_time; // The date-time the auth code was requested
    char *claimed_by;   // The handle of the user that claimed the auth code
    char *claim_time;   // The date-time the auth code was claimed
} api_auth_code_status_t;

/**
 * @brief Badge data result for the /badge endpoint
 */
typedef struct {
    int id;               // DB record ID for the badge
    char *badge_id;       // The badge ID (from the MAC address)
    char *handle;         // The username/handle for the badge user
    int xp;               // The user's current XP
    int level;            // The user's current level
    bool enabled;         // Whether the badge is enabled
    bool badge_team;      // Whether the badge belongs to a badge team member
    bool staff;           // Whether the badge is a staff badge
    bool blackbadge;      // Whether the badge is a black badge
    bool can_level;       // Whether the badge can level up other users
    char *community;      // The community the user is a part of (if any)
    int community_levels; // The number of levels the user has in their community
    bool is_savior;       // Whether the user is currently a savior
    int coins;            // The number of coins the user has
} api_badge_data_t;

/**
 * @brief Badge firmware result for the /badge/firmware_version endpoint
 */
typedef struct {
    char *setting; // Unused for now
    char *value;   // The latest firmware version (should be a semver string)
} api_firmware_data_t;

/**
 * @brief Join battle result from the /battle/join endpoint
 */
typedef struct {
    int battle_id; // The ID of the battle
} api_join_battle_t;

/**
 * @brief Tower status enum
 */
#define TOWER_STATUS_LIST \
    X(0x00, VULNERABLE)   \
    X(0x01, INVULNERABLE) \
    X(0x02, OFFLINE)      \
    X(0x03, DEFEATED)     \
    X(0xFF, UNKNOWN)
typedef enum {
#define X(val, name) TOWER_STATUS_##name = val,
    TOWER_STATUS_LIST
#undef X
} tower_status_t;
typedef struct {
    tower_status_t status;
    const char *name;
} tower_status_map_t;
extern const tower_status_map_t tower_status_map[];
tower_status_t get_tower_status(const char *status_str);

/**
 * @brief Tower status result for a single tower from the result field for the /badge/tower_status endpoint
 */
typedef struct {
    int id;                   // The tower ID
    char *name;               // The tower name
    char *location;           // The tower location
    int level;                // The tower level
    int health;               // The tower health
    int max_health;           // The tower max health
    char *boot_time;          // The tower boot time
    uint32_t ir_code;         // The tower IR code
    bool enabled;             // Whether the tower is enabled
    tower_status_t status;    // The tower status
    int players_in_range;     // The number of players in range of the tower
    int players_in_battle;    // The number of players battling the tower
    int players_joined_tower; // The number of players joined to the tower
    int players_disconnected; // The number of disconnected players
} api_tower_info_t;

/**
 * @brief Tower status results for all towers from the /badge/all_tower_status endpoint
 */
typedef struct {
    api_tower_info_t *towers; // Array of tower status results
    int count;                // Number of tower status results
} api_all_tower_info_t;

/**
 * @brief IR codes result from the result field for the /badge/ir_code endpoint
 */
#define IR_CODE_TYPE_LIST \
    X(0x00, TOWER)        \
    X(0x01, LEVELUP)      \
    X(0x02, SAVIOR)       \
    X(0x03, PVP)          \
    X(0x04, VENDING)      \
    X(0x05, AUTH)         \
    X(0x0F, MULTIFRAME)   \
    X(0xFF, UNKNOWN)
typedef enum {
#define X(val, name) IR_CODE_TYPE_##name = val,
    IR_CODE_TYPE_LIST
#undef X
} ir_code_type_t;
typedef struct {
    ir_code_type_t type;
    const char *name;
} ir_code_type_map_t;
extern const ir_code_type_map_t ir_code_type_map[];
ir_code_type_t get_ir_code_type(const char *type_str);
typedef struct {
    uint32_t code;       // The IR code
    bool is_valid;       // Whether the IR code is valid
    ir_code_type_t type; // The IR code type
    char *name;          // The IR code name
    char *response;      // The response message for the IR code
    int tower_id;        // The tower ID (if the IR code is for a tower)
} api_ir_code_t;
typedef struct {
    api_ir_code_t *ir_codes; // Array of IR codes
    int count;               // Number of IR codes
} api_ir_code_result_t;

/**
 * @brief Equip minibadge result from the /badge/equip endpoint
 */
#define MINIBADGE_BUFF_TYPE_LIST \
    X(0x00, NONE)                \
    X(0x01, JOYSTICK)            \
    X(0x02, LOOT)                \
    X(0x03, STRAT_SIMPLIFY)      \
    X(0x04, SPELLCHECK)          \
    X(0x05, QUAD_DAMAGE)         \
    X(0x06, GLASS_CANNON)        \
    X(0x07, SAVING_THROW)        \
    X(0x08, REVERSE_SHELL)       \
    X(0x09, HYDRA)               \
    X(0x0A, ZODIAKS_SANDALS)     \
    X(0x0B, GREED)
typedef enum {
#define X(val, name) MINIBADGE_BUFF_TYPE_##name = val,
    MINIBADGE_BUFF_TYPE_LIST
#undef X
} minibadge_buff_type_t;
typedef struct {
    minibadge_buff_type_t type;
    const char *name;
} minibadge_buff_type_map_t;
extern const minibadge_buff_type_map_t minibadge_buff_type_map[];
minibadge_buff_type_t get_minibadge_buff_type(const char *type_str);
typedef struct {
    char *slot;                      // The slot name (e.g. "slot1")
    char *name;                      // The minibadge name
    char *shortname;                 // The minibadge short name
    minibadge_buff_type_t buff_type; // The buff type
    float buff_value;                // The buff value
    bool valid;                      // Whether the minibadge is valid
    char *rewards;                   // The rewards message for the owning player (XP gained by lending the minibadge)
} api_minibadge_slot_info_t;
typedef struct {
    api_minibadge_slot_info_t slot1;
    api_minibadge_slot_info_t slot2;
} api_equip_minibadge_t;

/**
 * @brief Vend items result from the /vend/items endpoint
 */
typedef struct {
    int item_id;
    char *item_name;
    float item_price;
    int available_stock;
    bool purchased;
    bool sold_out;
    char *image_url;
    int version;
} api_vend_item_t;
typedef struct {
    api_vend_item_t *items; // Array of vend items
    int count;              // Number of vend items
} api_vend_items_t;

/**
 * @brief Send attack result from the /battle/attack endpoint
 */
typedef struct {
    int percent; // The percentage of the speed/accuracy against the global average for the stratagem length/count combination
} api_send_attack_t;

/**
 * @brief Battle status result from the /battle/status endpoint
 */
#define PLAYER_STATUS_LIST \
    X(0x00, JOINED)        \
    X(0x01, BATTLE)        \
    X(0x02, DISCONNECTED)  \
    X(0xFF, UNKNOWN)
typedef enum {
#define X(val, name) PLAYER_STATUS_##name = val,
    PLAYER_STATUS_LIST
#undef X
} player_status_t;
typedef struct {
    player_status_t status;
    const char *name;
} player_status_map_t;
extern const player_status_map_t player_status_map[];
player_status_t get_player_status(const char *status_str);
typedef struct {
    bool battle_active;            // Whether a battle is active
    int player_level;              // The player's level
    int player_hp;                 // The player's HP
    player_status_t player_status; // The player's status
    bool is_savior;                // Whether the player is currently a savior
    uint32_t tower_ir_code;        // The tower IR code
    int tower_level;               // The tower level
    int tower_health;              // The tower health
    int tower_max_health;          // The tower max health
    int players_in_range;          // The number of players in range of the tower
    int players_joined_tower;      // The number of players joined to the tower
    int players_in_battle;         // The number of players in battle
    int strategem_min;             // The minimum strategem value
    int strategem_max;             // The maximum strategem value
    int stratagem_amount;          // The amount of stratagems to send
    int players_disconnected;      // The number of disconnected players
    char **savior_handle;          // Array of savior handles
    int savior_handle_count;       // Number of savior handles
} api_battle_status_t;

/**
 * @brief After action report result from the /battle/aar endpoint
 */
typedef struct {
    int total_attacks;
    int total_damage;
    int xp_earned;
    int avg_performance;
    int failures;
    int coins_earned;
} api_after_action_report_t;

#ifdef __cplusplus
}
#endif
