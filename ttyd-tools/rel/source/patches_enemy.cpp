#include "patches_enemy.h"

#include "evt_cmd.h"
#include "mod.h"
#include "mod_state.h"
#include "patch.h"
#include "patches_battle.h"
#include "tot_generate_enemy.h"

#include <ttyd/battle.h>
#include <ttyd/battle_damage.h>
#include <ttyd/battle_database_common.h>
#include <ttyd/battle_enemy_item.h>
#include <ttyd/battle_event_cmd.h>
#include <ttyd/battle_sub.h>
#include <ttyd/battle_unit.h>
#include <ttyd/battle_unit_event.h>
#include <ttyd/evtmgr.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/item_data.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/system.h>

#include <cstdint>

namespace mod::infinite_pit {

namespace {

// For convenience.
using namespace ::ttyd::battle_database_common;
using namespace ::ttyd::battle_unit;

using ::ttyd::battle::BattleWork;
using ::ttyd::evtmgr::EvtEntry;
using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;

namespace BattleUnitType = ::ttyd::battle_database_common::BattleUnitType;
namespace ItemType = ::ttyd::item_data::ItemType;
namespace PartsAttribute_Flags = ::ttyd::battle_unit::PartsAttribute_Flags;

}

// Function hooks.
extern int32_t (*g_BtlUnit_Delete_trampoline)(BattleWorkUnit* unit);
extern BattleWorkUnit* (*g_BtlUnit_Entry_trampoline)(BattleUnitSetup*);
extern bool (*g_BattleCheckEndUnitInitEvent_trampoline)(BattleWork*);
extern int32_t (*g_btlevtcmd_SetEventAttack_trampoline)(EvtEntry*, bool);
extern int32_t (*g_btlevtcmd_ConsumeItem_trampoline)(EvtEntry*, bool);
extern int32_t (*g_btlevtcmd_GetConsumeItem_trampoline)(EvtEntry*, bool);
extern int32_t (*g_BtlUnit_GetCoin_trampoline)(BattleWorkUnit*);
extern void* (*g_BattleEnemyUseItemCheck_trampoline)(BattleWorkUnit*);
// Patch addresses.
extern const int32_t g_BtlUnit_EnemyItemCanUseCheck_Patch_SkipCheck;

namespace enemy {
    
namespace {

// Global variable for the last type of item consumed;
// this is necessary to allow enemies to use cooked items.
int32_t g_EnemyItem = 0;

void ApplyMidbossStats(BattleWorkUnit* unit) {
    // Unit work 3 is used as sentinel in initialization for 'is midboss'.
    if (unit->unit_work[3] == 1) {
        unit->unit_work[3] = 0;
        
        unit->current_hp *= 6;
        unit->base_max_hp *= 6;
        
        // Apply permanent Huge status.
        unit->size_change_strength = 1;
        unit->size_change_turns = 100;

        // Change held item display offset to be more easily visible.
        unit->held_item_base_offset.x *= 2;
        unit->held_item_base_offset.y *= 2;
        
        // Apply "MIDBOSS" status so Huge status can be ended if necessary.
        unit->status_flags |= BattleUnitStatus_Flags::MIDBOSS;
    }
}

void RemoveMidbossWeaknessAttributes() {
    auto* battleWork = ttyd::battle::g_BattleWork;
    for (int32_t i = 0; i < 64; ++i) {
        auto* unit = ttyd::battle::BattleGetUnitPtr(battleWork, i);
        if (unit && (unit->status_flags & BattleUnitStatus_Flags::MIDBOSS)) {
            // Turn off weaknesses that incapacitate the enemy.
            for (BattleWorkUnitPart* part = unit->parts; 
                 part != nullptr; part = part->next_part) {
                 part->part_attribute_flags &= ~(
                    PartsAttribute_Flags::WINGED |
                    PartsAttribute_Flags::SHELLED |
                    PartsAttribute_Flags::BOMB_FLIPPABLE
                 );
            }
        }
    }
}

void AlterUnitKindParams(BattleUnitKind* unit) {
    // If not an enemy, nothing to change.
    if (unit->unit_type > BattleUnitType::BONETAIL) return;
    
    int32_t hp, level, coinlvl;
    if (!tot::GetEnemyStats(
        unit->unit_type, &hp, nullptr, nullptr, &level, &coinlvl)) return;
    unit->max_hp = hp;
    
    if (ttyd::mario_pouch::pouchGetPtr()->level >= 99) {
        // Assign enemies a high level so you can't Gale Force them to oblivion.
        unit->level = 99;
        unit->bonus_exp = 0;
    } else if (level >= 0) {
        unit->level = level;
        unit->bonus_exp = 0;
    } else {
        // If negative, give it as bonus EXP instead (to avoid level overflow).
        unit->level = ttyd::mario_pouch::pouchGetPtr()->level + 1;
        unit->bonus_exp = -level;
    }
    
    // Give coins equal to the underlying level (2~10).
    unit->base_coin = coinlvl;
    unit->bonus_coin = 0;
    
    // Additional global changes for enemies in this mod.
    unit->itemsteal_param = 20;
}

// Runs extra code on consuming an item and getting the item to be consumed,
// allowing for enemies to use generic cooking items.
void EnemyConsumeItem(ttyd::evtmgr::EvtEntry* evt) {
    auto* battleWork = ttyd::battle::g_BattleWork;
    int32_t id = evtGetValue(evt, evt->evtArguments[0]);
    id = ttyd::battle_sub::BattleTransID(evt, id);
    auto* unit = ttyd::battle::BattleGetUnitPtr(battleWork, id);
    if (unit->current_kind <= BattleUnitType::BONETAIL) {
        g_EnemyItem = unit->held_item;
    }
}
// Returns true if the evt was run by an enemy.
bool GetEnemyConsumeItem(ttyd::evtmgr::EvtEntry* evt) {
    auto* battleWork = ttyd::battle::g_BattleWork;
    BattleWorkUnit* unit = nullptr;
    if (evt->wActorThisPtr) {
        unit = ttyd::battle::BattleGetUnitPtr(
            battleWork,
            reinterpret_cast<uint32_t>(evt->wActorThisPtr));
        if (unit->current_kind <= BattleUnitType::BONETAIL) {
            evtSetValue(evt, evt->evtArguments[0], g_EnemyItem);
            return true;
        }
    }
    return false;
}

void* EnemyUseAdditionalItemsCheck(BattleWorkUnit* unit) {
    switch (unit->held_item) {
        // Items that aren't normally usable but work with no problems:
        case ItemType::COCONUT:
        case ItemType::COURAGE_MEAL:
        case ItemType::EGG_BOMB:
        case ItemType::COCONUT_BOMB:
        case ItemType::ZESS_DYNAMITE:
        case ItemType::HOT_SAUCE:
        case ItemType::SPITE_POUCH:
        case ItemType::KOOPA_CURSE:
        case ItemType::ELECTRO_POP:
        case ItemType::LOVE_PUDDING:
        case ItemType::PEACH_TART:
        case ItemType::SPACE_FOOD:
        // Try to use for status effects even if not needed for healing:
        case ItemType::SPICY_PASTA:
        case ItemType::FRUIT_PARFAIT:
        // Additional items (would not have the desired effect without patches):
        case ItemType::POISON_SHROOM:
        case ItemType::POINT_SWAP:
        case ItemType::TRIAL_STEW:
        case ItemType::TRADE_OFF:
            return ttyd::battle_enemy_item::_check_attack_item(unit);
        case ItemType::HEALTHY_SALAD:
            return ttyd::battle_enemy_item::_check_status_recover_item(unit);
        // Explicitly not allowed:
        case ItemType::MYSTIC_EGG:
        case ItemType::METEOR_MEAL:
        case ItemType::FRIGHT_MASK:
        case ItemType::MYSTERY:
        default:
            return nullptr;
    }
}

}
    
void ApplyFixedPatches() {
    // Alter unit kind params in accordance with scaled stats, and apply
    // increased stats for midbosses.
    g_BtlUnit_Entry_trampoline = patch::hookFunction(
        ttyd::battle_unit::BtlUnit_Entry, [](BattleUnitSetup* unit_setup) {
            AlterUnitKindParams(unit_setup->unit_kind_params);
            BattleWorkUnit* unit = g_BtlUnit_Entry_trampoline(unit_setup);
            ApplyMidbossStats(unit);
            return unit;
        });

    // Track enemy kills on deletion.
    g_BtlUnit_Delete_trampoline = patch::hookFunction(
        ttyd::battle_unit::BtlUnit_Delete, [](BattleWorkUnit* unit) {
            int32_t tattle_idx = tot::GetCustomTattleIndex(unit->true_kind);
            if (tattle_idx >= 0 &&
                g_Mod->state_.GetOption(tot::STAT_PERM_ENEMY_KILLS, tattle_idx) < 9999) {
                g_Mod->state_.ChangeOption(tot::STAT_PERM_ENEMY_KILLS, 1, tattle_idx);
            }
            // Run original logic.
            return g_BtlUnit_Delete_trampoline(unit);
        });
        
    g_btlevtcmd_ConsumeItem_trampoline = patch::hookFunction(
        ttyd::battle_event_cmd::btlevtcmd_ConsumeItem,
        [](EvtEntry* evt, bool isFirstCall) {
            EnemyConsumeItem(evt);
            return g_btlevtcmd_ConsumeItem_trampoline(evt, isFirstCall);
        });
        
    g_btlevtcmd_GetConsumeItem_trampoline = patch::hookFunction(
        ttyd::battle_event_cmd::btlevtcmd_GetConsumeItem,
        [](EvtEntry* evt, bool isFirstCall) {
            if (GetEnemyConsumeItem(evt)) return 2;
            return g_btlevtcmd_GetConsumeItem_trampoline(evt, isFirstCall);
        });
        
    g_BattleEnemyUseItemCheck_trampoline = patch::hookFunction(
        ttyd::battle_enemy_item::BattleEnemyUseItemCheck,
        [](BattleWorkUnit* unit) {
            void* evt_code = g_BattleEnemyUseItemCheck_trampoline(unit);
            if (!evt_code) {
                evt_code = EnemyUseAdditionalItemsCheck(unit);
            }
            return evt_code;
        });
        
    // Disable the check for enemies only holding certain types of items.
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_BtlUnit_EnemyItemCanUseCheck_Patch_SkipCheck),
        0x60000000U /* nop */);
        
    // Assign midbosses a wrapper attack script.
    g_btlevtcmd_SetEventAttack_trampoline = patch::hookFunction(
        ttyd::battle_event_cmd::btlevtcmd_SetEventAttack,
        [](EvtEntry* evt, bool isFirstCall) {
            auto* battleWork = ttyd::battle::g_BattleWork;
            int32_t id = evtGetValue(evt, evt->evtArguments[0]);
            id = ttyd::battle_sub::BattleTransID(evt, id);
            auto* unit = ttyd::battle::BattleGetUnitPtr(battleWork, id);
            void* script = (void*)evt->evtArguments[1];
            if (unit->status_flags & BattleUnitStatus_Flags::MIDBOSS) {
                script = tot::GetMidbossAttackScript(script);
            }
            unit->attack_evt_code = script;
            unit->battle_menu_state = 0;
            return 2;
        });
    
    // Hook 'init events finished' check to clear midboss weakness attributes.
    g_BattleCheckEndUnitInitEvent_trampoline = patch::hookFunction(
        ttyd::battle_unit_event::BattleCheckEndUnitInitEvent,
        [](BattleWork* battleWork) {
            bool result = g_BattleCheckEndUnitInitEvent_trampoline(battleWork);
            if (result) {
                RemoveMidbossWeaknessAttributes();
            }
            return result;
        });
    
    // Override the number of coins earned from an enemy.
    g_BtlUnit_GetCoin_trampoline = patch::hookFunction(
        ttyd::battle_unit::BtlUnit_GetCoin, [](BattleWorkUnit* unit) {
            int32_t coins = g_BtlUnit_GetCoin_trampoline(unit);
            // 5x coin multiplier for midbosses.
            if (unit->status_flags & BattleUnitStatus_Flags::MIDBOSS) coins *= 5;
            // Extra coins from Trade Off.
            coins += unit->pad_00f;
            return coins;
        });
}

}  // namespace enemy
}  // namespace mod::infinite_pit