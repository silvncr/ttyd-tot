#pragma once

#include "evt_cmd.h"

#include <ttyd/evtmgr.h>

#include <cstdint>


namespace ttyd::battle_damage {

struct CounterattackWork;

}
namespace ttyd::battle_database_common {

struct BattleWeapon;

}
namespace ttyd::battle_unit {

struct BattleWorkUnit;
struct BattleWorkUnitPart;

}

namespace mod::infinite_pit::battle {

// Apply patches to various battle features.
void ApplyFixedPatches();

// Overrides the default target audience amount to be based on Pit progression.
void SetTargetAudienceAmount();
// Applies the option to change the SP amount regained from attacks.
double ApplySpRegenMultiplier(double base_regen);

// Calculates the base damage for an attack, replacing the original TTYD func.
int32_t CalculateBaseDamage(
    ttyd::battle_unit::BattleWorkUnit* attacker, 
    ttyd::battle_unit::BattleWorkUnit* target, 
    ttyd::battle_unit::BattleWorkUnitPart* part,
    ttyd::battle_database_common::BattleWeapon* weapon, 
    uint32_t* unk0, uint32_t unk1);

// Calculates the damage dealt by a counterattack.
void CalculateCounterDamage(
    ttyd::battle_damage::CounterattackWork* counter_work,
    ttyd::battle_unit::BattleWorkUnit* attacker,
    ttyd::battle_unit::BattleWorkUnit* target,
    ttyd::battle_unit::BattleWorkUnitPart* part,
    int32_t damage_dealt);

// Toggles off "Scoped" status for the target if attacked by the player.
void ToggleScopedStatus(
    ttyd::battle_unit::BattleWorkUnit* attacker, 
    ttyd::battle_unit::BattleWorkUnit* target,
    ttyd::battle_database_common::BattleWeapon* weapon);

// Queues a custom status announcement message.
void QueueCustomStatusMessage(
    ttyd::battle_unit::BattleWorkUnit* unit, const char* announce_msg);

// Gets the party switch cost in FP (0, or 1+ with Quick Change).
int32_t GetPartySwitchCost();
// Resets the party switch cost at the start of an encounter.
void ResetPartySwitchCost();
// Signals that a party switch should cost FP unless cancelled by Confusion.
void SignalPlayerInitiatedPartySwitch();

// Applies a custom status effect to the target.
// Params: unit, part, status_flag, color1 & color2 (rgb), sfx, announce_msg
EVT_DECLARE_USER_FUNC(evtTot_ApplyCustomStatus, 7)

}