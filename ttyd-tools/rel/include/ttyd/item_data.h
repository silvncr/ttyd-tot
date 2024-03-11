#pragma once

#include <cstdint>

namespace ttyd::battle_database_common {
struct BattleWeapon;
}

namespace ttyd::item_data {

// Types of key items, items, and badges.
namespace ItemType {
    enum e {
        INVALID_ITEM = 0,
        
        // Key items / abilities (excluding Shine Sprite).
        STRANGE_SACK = 0x0001,
        PAPER_CURSE,
        TUBE_CURSE,
        PLANE_CURSE,
        BOAT_CURSE,
        BOOTS,
        SUPER_BOOTS,
        ULTRA_BOOTS,
        HAMMER,
        SUPER_HAMMER,
        ULTRA_HAMMER,
        CASTLE_KEY_000C,
        CASTLE_KEY_000D,
        CASTLE_KEY_000E,
        CASTLE_KEY_000F,
        RED_KEY_0010,
        BLUE_KEY_0011,
        STORAGE_KEY_0012,
        STORAGE_KEY_0013,
        GROTTO_KEY_0014,
        SHOP_KEY_0015,
        STEEPLE_KEY_0016,
        STEEPLE_KEY_0017,
        STATION_KEY_0018,
        STATION_KEY_0019,
        ELEVATOR_KEY_001A,
        ELEVATOR_KEY_001B,
        ELEVATOR_KEY_001C,
        CARD_KEY_001D,
        CARD_KEY_001E,
        CARD_KEY_001F,
        CARD_KEY_0020,
        BLACK_KEY_0021,
        BLACK_KEY_0022,
        BLACK_KEY_0023,
        BLACK_KEY_0024,
        STAR_KEY_0025,
        PALACE_KEY_0026,
        PALACE_KEY_0027,
        PALACE_KEY_0028,
        PALACE_KEY_0029,
        PALACE_KEY_002A,
        PALACE_KEY_002B,
        PALACE_KEY_002C,
        PALACE_KEY_002D,
        PALACE_KEY_002E,
        PALACE_KEY_002F,
        PALACE_KEY_0030,
        HOUSE_KEY_0031,
        MAGICAL_MAP,
        CONTACT_LENS,
        BLIMP_TICKET,
        TRAIN_TICKET,
        MAILBOX_SP,
        SUPER_LUIGI,
        SUPER_LUIGI_2,
        SUPER_LUIGI_3,
        SUPER_LUIGI_4,
        SUPER_LUIGI_5,
        COOKBOOK,
        MOON_STONE,
        SUN_STONE,
        NECKLACE,
        PUNI_ORB,
        CHAMPS_BELT,
        POISONED_CAKE,
        SUPERBOMBOMB,
        THE_LETTER_P,
        OLD_LETTER,
        CHUCKOLA_COLA,
        SKULL_GEM,
        GATE_HANDLE,
        WEDDING_RING,
        GALLEY_POT,
        GOLD_RING,
        SHELL_EARRINGS,
        AUTOGRAPH,
        RAGGED_DIARY,
        BLANKET,
        VITAL_PAPER,
        BRIEFCASE,
        GOLDBOB_GUIDE,
        INVALID_ITEM_PAPER_0053,
        INVALID_ITEM_PAPER_0054,
        COG,
        DATA_DISK,
        SHINE_SPRITE,       // 0x0057
        ULTRA_STONE,
        INVALID_ITEM_BOWSER_MEAT_0059,
        INVALID_ITEM_MARIO_POSTER_005A,
        SPECIAL_CARD,
        PLATINUM_CARD,
        GOLD_CARD,
        SILVER_CARD,
        BOX,
        MAGICAL_MAP_LARGE,
        DUBIOUS_PAPER,
        ROUTING_SLIP,
        WRESTLING_MAG,
        PRESENT,
        BLUE_POTION,
        RED_POTION,
        ORANGE_POTION,
        GREEN_POTION,
        INVALID_ITEM_STAR_FN0OW_0069,
        LOTTERY_PICK,
        BATTLE_TRUNKS,
        UP_ARROW,
        PACKAGE,
        ATTACK_FX_B_KEY_ITEM,
        INVALID_ITEM_006F,
        INVALID_ITEM_0070,
        INVALID_ITEM_0071,
        DIAMOND_STAR,
        EMERALD_STAR,
        GOLD_STAR,
        RUBY_STAR,
        SAPPHIRE_STAR,
        GARNET_STAR,
        CRYSTAL_STAR,
        
        // Currency / pickups.
        COIN = 0x0079,
        PIANTA,
        HEART_PICKUP,
        FLOWER_PICKUP,
        STAR_PIECE,         // 0x007d
        
        // Items.
        GOLD_BAR = 0x007E,
        GOLD_BAR_X3,
        THUNDER_BOLT,       // 0x0080
        THUNDER_RAGE,
        SHOOTING_STAR,
        ICE_STORM,
        FIRE_FLOWER,
        EARTH_QUAKE,
        BOOS_SHEET,
        VOLT_SHROOM,
        REPEL_CAPE,
        RUIN_POWDER,
        SLEEPY_SHEEP,
        POW_BLOCK,
        STOPWATCH,
        DIZZY_DIAL,
        POWER_PUNCH,
        COURAGE_SHELL,
        HP_DRAIN_ITEM,
        TRADE_OFF,          // 0x0091    
        MINI_MR_MINI,
        MR_SOFTENER,
        MUSHROOM,
        SUPER_SHROOM,
        ULTRA_SHROOM,
        LIFE_SHROOM,
        DRIED_SHROOM,
        TASTY_TONIC,
        HONEY_SYRUP,
        MAPLE_SYRUP,
        JAMMIN_JELLY,
        SLOW_SHROOM,
        GRADUAL_SYRUP,
        HOT_DOG,
        CAKE,
        POINT_SWAP,         // 0x00a1    
        FRIGHT_MASK,
        MYSTERY,
        INN_COUPON,
        WHACKA_BUMP,        // 0x00a5
        COCONUT,
        DRIED_BOUQUET,
        MYSTIC_EGG,
        GOLDEN_LEAF,
        KEEL_MANGO,
        FRESH_PASTA,
        CAKE_MIX,
        HOT_SAUCE,
        TURTLEY_LEAF,
        HORSETAIL,
        PEACHY_PEACH,
        SPITE_POUCH,        // 0x00b1
        KOOPA_CURSE,
        
        // Recipe items.
        SHROOM_FRY = 0x00B3,
        SHROOM_ROAST,
        SHROOM_STEAK,
        MISTAKE,
        HONEY_SHROOM,
        MAPLE_SHROOM,
        JELLY_SHROOM,
        HONEY_SUPER,
        MAPLE_SUPER,
        JELLY_SUPER,
        HONEY_ULTRA,
        MAPLE_ULTRA,
        JELLY_ULTRA,
        SPICY_SOUP,
        ZESS_DINNER,
        ZESS_SPECIAL,
        ZESS_DELUXE,
        ZESS_DYNAMITE,
        ZESS_TEA,
        SPACE_FOOD,
        ICICLE_POP,
        ZESS_FRAPPE,
        SNOW_BUNNY,
        COCONUT_BOMB,
        COURAGE_MEAL,
        SHROOM_CAKE,
        SHROOM_CREPE,
        MOUSSE_CAKE,
        FRIED_EGG,
        FRUIT_PARFAIT,
        EGG_BOMB,
        INK_PASTA,
        SPAGHETTI,
        SHROOM_BROTH,
        POISON_SHROOM,
        CHOCO_CAKE,
        MANGO_DELIGHT,
        LOVE_PUDDING,
        METEOR_MEAL,
        TRIAL_STEW,
        COUPLES_CAKE,
        INKY_SAUCE,
        OMELETTE_MEAL,
        KOOPA_TEA,
        KOOPASTA,
        SPICY_PASTA,
        HEARTFUL_CAKE,
        PEACH_TART,
        ELECTRO_POP,
        FIRE_POP,
        HONEY_CANDY,
        COCO_CANDY,
        JELLY_CANDY,
        ZESS_COOKIE,
        HEALTHY_SALAD,
        KOOPA_BUN,
        FRESH_JUICE,
        
        // Audience weapons.
        AUDIENCE_CAN = 0x00EC,
        AUDIENCE_ROCK,
        AUDIENCE_BONE,
        AUDIENCE_HAMMER,
        
        // Badges (a few P variants unused).
        POWER_JUMP = 0x00F0,
        MULTIBOUNCE,
        POWER_BOUNCE,
        TORNADO_JUMP,
        SHRINK_STOMP,
        SLEEPY_STOMP,
        SOFT_STOMP,
        POWER_SMASH,
        QUAKE_HAMMER,
        HAMMER_THROW,
        PIERCING_BLOW,
        HEAD_RATTLE,
        FIRE_DRIVE,
        ICE_SMASH,
        DOUBLE_DIP,
        DOUBLE_DIP_P,
        CHARGE,
        CHARGE_P,
        SUPER_APPEAL,
        SUPER_APPEAL_P,
        POWER_PLUS,
        POWER_PLUS_P,
        P_UP_D_DOWN,
        P_UP_D_DOWN_P,
        ALL_OR_NOTHING,
        ALL_OR_NOTHING_P,
        MEGA_RUSH,
        MEGA_RUSH_P,
        POWER_RUSH,
        POWER_RUSH_P,
        P_DOWN_D_UP,
        P_DOWN_D_UP_P,
        LAST_STAND,
        LAST_STAND_P,
        DEFEND_PLUS,
        DEFEND_PLUS_P,
        DAMAGE_DODGE,
        DAMAGE_DODGE_P,
        HP_PLUS,
        HP_PLUS_P,
        FP_PLUS,
        FLOWER_SAVER,
        FLOWER_SAVER_P,
        ICE_POWER,
        SPIKE_SHIELD,
        FEELING_FINE,
        FEELING_FINE_P,
        ZAP_TAP,
        DOUBLE_PAIN,
        JUMPMAN,
        HAMMERMAN,
        RETURN_POSTAGE,
        HAPPY_HEART,
        HAPPY_HEART_P,
        HAPPY_FLOWER,
        HP_DRAIN,
        HP_DRAIN_P,
        FP_DRAIN,
        FP_DRAIN_P,
        CLOSE_CALL,
        CLOSE_CALL_P,
        PRETTY_LUCKY,
        PRETTY_LUCKY_P,
        LUCKY_DAY,
        LUCKY_DAY_P,
        REFUND,
        PITY_FLOWER,
        PITY_FLOWER_P,
        QUICK_CHANGE,
        PEEKABOO,
        TIMING_TUTOR,
        HEART_FINDER,
        FLOWER_FINDER,
        MONEY_MONEY,
        ITEM_HOG,
        ATTACK_FX_R,
        ATTACK_FX_B,
        ATTACK_FX_G,
        ATTACK_FX_Y,
        ATTACK_FX_P,
        CHILL_OUT,
        FIRST_ATTACK,
        BUMP_ATTACK,
        SLOW_GO,
        SIMPLIFIER,
        UNSIMPLIFIER,
        LUCKY_START,
        L_EMBLEM,
        W_EMBLEM,
        
        // Unused badges.
        TRIPLE_DIP = 0x149,
        LUCKY_START_P,
        AUTO_COMMAND_BADGE,
        MEGA_JUMP,
        MEGA_SMASH,
        MEGA_QUAKE,
        SQUARE_DIAMOND_BADGE,
        SQUARE_DIAMOND_BADGE_P,
        SUPER_CHARGE,
        SUPER_CHARGE_P,
        
        MAX_ITEM_TYPE,
    };
}

namespace ItemUseLocation_Flags {
    enum e {
        kShop = 1,
        kBattle = 2,
        kField = 4,
    };
}

struct ItemData {
    const char*     id;
    const char*     name;
    const char*     description;
    const char*     menu_description;
    int16_t         usable_locations;
    int16_t         type_sort_order;
    int16_t         buy_price;
    int16_t         discount_price;
    int16_t         star_piece_price;
    int16_t         sell_price;
    int8_t          bp_cost;
    int8_t          hp_restored;
    int8_t          fp_restored;
    int8_t          sp_restored;  // Only works on field
    int16_t         icon_id;
    int16_t         unk_022;
    ttyd::battle_database_common::BattleWeapon* weapon_params;
} __attribute__((__packed__));

static_assert(sizeof(ItemData) == 0x28);

extern "C" {

extern ItemData itemDataTable[0x153];

}

}