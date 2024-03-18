#include "tot_state.h"

#include "common_functions.h"
#include "common_types.h"
#include "evt_cmd.h"
#include "mod.h"

#include <gc/OSTime.h>
#include <third_party/fasthash.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/system.h>

#include <cinttypes>
#include <cstdio>
#include <cstring>

namespace mod::tot {

namespace {

using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;

void GetOptionParts(
    uint32_t v, int32_t* t, int32_t* x, int32_t* y, int32_t* a, int32_t* b) {
    *t = GetShiftedBitMask(v, 28, 31);
    // Get the full upper 12 bits if STAT_, otherwise just the lower eight.
    *x = GetShiftedBitMask(v, 20, *t < 4 ? 31 : 27);
    *y = GetShiftedBitMask(v, 16, 19);
    *a = GetShiftedBitMask(v, 8, 15);
    *b = GetShiftedBitMask(v, 0, 7);
}

}  // namespace

// Loading / saving functions.
// bool StateManager::Load(TotSaveSlotData* save);
// void StateManager::Save(TotSaveSlotData* save);
// TotSaveSlotData* StateManager::GetBackupSave();

void StateManager::InitDefaultOptions() {
    // TODO: Seed randomly, or allow user to change.
    seed_ = 417;
    
    // TODO: Once enemy-less floor 0 is implemented, start at 0.
    floor_ = 1;
    
    // Set stat upgrades to base # of levels.
    hp_level_ = 2;
    hp_p_level_ = 2;
    fp_level_ = 1;
    bp_level_ = 1;
    num_sack_upgrades_ = 0;
    
    memset(option_flags_, 0, sizeof(option_flags_));
    memset(option_bytes_, 0, sizeof(option_bytes_));
    // Set non-zero default values to their proper values.
    SetOption(OPTVAL_DIFFICULTY_FULL);
    SetOption(OPTVAL_STARTER_ITEMS_BASIC);
    SetOption(OPTNUM_MARIO_HP, 5);
    SetOption(OPTNUM_MARIO_FP, 5);
    SetOption(OPTNUM_MARIO_BP, 5);
    SetOption(OPTNUM_PARTNER_HP, 5);
    SetOption(OPTNUM_ENEMY_HP, 100);
    SetOption(OPTNUM_ENEMY_ATK, 100);
}

bool StateManager::SetOption(uint32_t option, int32_t value) {
    int32_t t, x, y, a, b;
    GetOptionParts(option, &t, &x, &y, &a, &b);
    
    switch (t) {
        case TYPE_OPT:
        case TYPE_OPTVAL: {
            if (t == TYPE_OPTVAL) {
                // We know that the value must be in range.
                value = b;
            } else if (value < a || value > b) {
                // Value out of range for this flag, leave unchanged.
                return false;
            }
            uint32_t* ptr = option_flags_ + (x >> 5);
            const uint32_t start_bit = x & 31;
            const uint32_t mask = GetBitMask(start_bit, start_bit + y - 1);
            *ptr = (*ptr & ~mask) | (value << start_bit);
            return true;
        }
        case TYPE_OPTNUM: {
            value /= a;
            if (value < y || value > b) return false;
            option_bytes_[x] = value;
            return true;
        }
        case TYPE_FLAGS_ACHIEVEMENT:
        case TYPE_FLAGS_MOVE_ENCOUNTERED:
        case TYPE_FLAGS_ITEM_ENCOUNTERED:
        case TYPE_FLAGS_ITEM_PURCHASED:
        case TYPE_FLAGS_OPT_UNLOCKED: {
            // Can use either value or manually bit-or'd "b" for setting.
            if (value == 0) value = b;
            // Check for values outside valid range.
            if (value < 0 || value >= 256) return false;
            if (t != TYPE_FLAGS_ITEM_ENCOUNTERED && 
                t != TYPE_FLAGS_ITEM_PURCHASED && value >= 128) return false;
            
            uint32_t* ptr;
            switch (t) {
                case TYPE_FLAGS_ACHIEVEMENT: {
                    ptr = achievement_flags_ + (value >> 5);
                    break;
                }
                case TYPE_FLAGS_MOVE_ENCOUNTERED: {
                    ptr = move_encountered_flags_ + (value >> 5);
                    break;
                }
                case TYPE_FLAGS_ITEM_ENCOUNTERED: {
                    ptr = item_encountered_flags_ + (value >> 5);
                    break;
                }
                case TYPE_FLAGS_ITEM_PURCHASED: {
                    ptr = item_purchased_flags_ + (value >> 5);
                    break;
                }
                case TYPE_FLAGS_OPT_UNLOCKED: {
                    ptr = option_unlocked_flags_ + (value >> 5);
                    break;
                }
            }
            *ptr |= (1 << (value & 31));
            return true;
        }
        // Play stats.
        default: {
            if (a == 1) {
                // Clamp number to b digits long.
                if (b < 1 || b > 9) return false;
                static const constexpr int32_t powers_of_10[] = {
                    1, 10, 100, 1000, 10'000, 100'000, 1'000'000, 10'000'000,
                    100'000'000, 1'000'000'000
                };
                if (value >= powers_of_10[b]) value = powers_of_10[b] - 1;
                if (value <= -powers_of_10[b]) value = -(powers_of_10[b] - 1);
            }
            uint8_t* ptr = play_stats_ + x;
            uint32_t uint_val = static_cast<uint32_t>(value);
            for (int32_t i = y - 1; i >= 0; --i) {
                ptr[i] = uint_val & 0xff;
                uint_val >>= 8;
            }
            return true;
        }
    }
}

bool StateManager::ChangeOption(uint32_t option, int32_t change) {
    int32_t t, x, y, a, b;
    GetOptionParts(option, &t, &x, &y, &a, &b);
    if (t == TYPE_OPTNUM) change *= a;
    int32_t value = GetOption(option);
    return SetOption(option, value + change);
}

// Gets the numeric value of options, play stats, achievements, etc.
// 'value' is only used as a parameter for option types that require it.
int32_t StateManager::GetOption(uint32_t option, int32_t value) const {
    int32_t t, x, y, a, b;
    GetOptionParts(option, &t, &x, &y, &a, &b);
    // OPTVALs are constants, so this doesn't make sense to request.
    if (t == TYPE_OPTVAL) return -1;
    // FLAGS can use value or | with the base flag type.
    if (t >= TYPE_FLAGS_ACHIEVEMENT) {
        x = value == 0 ? b : value;
    }
    
    const uint32_t* flag_ptr = nullptr;
    const uint8_t* byte_ptr = nullptr;
    switch (t) {
        case TYPE_OPT: {
            flag_ptr = option_flags_ + (x >> 5);
            break;
        }
        case TYPE_FLAGS_ACHIEVEMENT: {
            flag_ptr = achievement_flags_ + (x >> 5);
            break;
        }
        case TYPE_FLAGS_MOVE_ENCOUNTERED: {
            flag_ptr = move_encountered_flags_ + (x >> 5);
            break;
        }
        case TYPE_FLAGS_ITEM_ENCOUNTERED: {
            flag_ptr = item_encountered_flags_ + (x >> 5);
            break;
        }
        case TYPE_FLAGS_ITEM_PURCHASED: {
            flag_ptr = item_purchased_flags_ + (x >> 5);
            break;
        }
        case TYPE_FLAGS_OPT_UNLOCKED: {
            flag_ptr = option_unlocked_flags_ + (x >> 5);
            break;
        }
        case TYPE_OPTNUM: {
            byte_ptr = option_bytes_ + x;
            break;
        }
        default: {
            byte_ptr = play_stats_ + x;
            break;
        }
    }
    
    if (flag_ptr) {
        const uint32_t start_bit = x & 31;
        return GetShiftedBitMask(*flag_ptr, start_bit, start_bit + y - 1);
    } else if (t == TYPE_OPTNUM) {
        return *byte_ptr * a;
    } else {
        // Start with all 0 or 1 bits based on sign of value.
        uint32_t uint_val = (*byte_ptr & 0x80) ? ~0 : 0;
        for (int32_t i = 0; i < y; ++i) {
            uint_val = (uint_val << 8) + *byte_ptr++;
        }
        return static_cast<int32_t>(uint_val);
    }
    // Should never be reachable.
    return -1;
}

int32_t StateManager::GetOptionValue(uint32_t option) const {
    int32_t t, x, y, a, b;
    GetOptionParts(option, &t, &x, &y, &a, &b);
    // Only TYPE_OPT has OPTVAL values.
    if (t != TYPE_OPT) return -1;
    
    const uint32_t word = option_flags_[x >> 5];
    const uint32_t start_bit = x & 31;
    const int32_t value = GetShiftedBitMask(word, start_bit, start_bit + y - 1);
    return (TYPE_OPT << 28) | (option & 0x0fff'0000) | value;
}

bool StateManager::CheckOptionValue(uint32_t option_value) const {
    int32_t t, x, y, a, b;
    GetOptionParts(option_value, &t, &x, &y, &a, &b);
    if (t != TYPE_OPTVAL) return false;
    
    const uint32_t word = option_flags_[x >> 5];
    const uint32_t start_bit = x & 31;
    const int32_t value = GetShiftedBitMask(word, start_bit, start_bit + y - 1);
    return value == b;
}

// Gets menu information (raw strings, not msg keys) for a given option.
// void StateManager::GetOptionStrings(
//      int32_t option, char* name_buf, char* value_buf, int32_t* cost,
//      bool* unlocked, bool* default, bool* affects_seeding) const;

// Returns a string representing the current options encoded.
// const char* StateManager::GetEncodedOptions() const;

void StateManager::IncrementFloor(int32_t change) {
    floor_ = Clamp(floor_ + change, 0, 64);
    
    // Clear RNG state values that should reset every floor.
    for (int32_t i = RNG_ENEMY; i <= RNG_REWARD; ++i) rng_states_[i] = 0;
    
    auto& pouch = *ttyd::mario_pouch::pouchGetPtr();
    // Set stage rank based on floor.
    pouch.rank = (floor_ - 1) / 16;
    // Display the current floor in place of Star Points, since they're unused.
    pouch.star_points = floor_;
}

// Functions for time-tracking...
// void StateManager::StartTimer();
// void StateManager::UpdateTimer();

void StateManager::ToggleIGT(bool toggle) {
    igt_active_ = toggle;
}

// Clear play stats, timers, etc. from current run.
// void StateManager::ClearRunStats();

// Fetches a random value from the desired sequence (using the RngSequence
// enum), returning a value in the range [0, range). If `sequence` is not
// a valid enum value, returns a random value using ttyd::system::irand().
uint32_t StateManager::Rand(uint32_t range, int32_t sequence) {
    if (sequence > RNG_VANILLA && sequence < RNG_SEQUENCE_MAX) {
        uint32_t data[2] = { 0, 0 };
        uint16_t* seq_val = rng_states_ + sequence;
        // Include the sequence id and current position, so the beginnings of
        // different sequences can't end up identical.
        // (e.g. chest random badge rewards + first floor's enemy items)
        data[0] = (*seq_val)++ | (sequence << 16);
        switch (sequence) {
            case RNG_ENEMY:
            case RNG_ENEMY_ITEM:
            case RNG_ENEMY_CONDITION:
            case RNG_ENEMY_CONDITION_ITEM:
            case RNG_NPC_TYPE:
            case RNG_NPC_OPTIONS:
            case RNG_NPC_RESERVED: {
                data[1] = floor_;
                break;
            }
            default:
                break;
        }
        return third_party::fasthash64(data, sizeof(data), seed_) % range;
    }
    return ttyd::system::irand(range);
}

EVT_DEFINE_USER_FUNC(evtTot_IncrementFloor) {
    int32_t change = evtGetValue(evt, evt->evtArguments[0]);
    infinite_pit::g_Mod->state_.IncrementFloor(change);
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_GetFloor) {
    int32_t floor = infinite_pit::g_Mod->state_.floor_;
    evtSetValue(evt, evt->evtArguments[0], floor);
    return 2;
}

}  // namespace mod::tot