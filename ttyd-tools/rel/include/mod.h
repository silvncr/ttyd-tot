#pragma once

#include "mod_state.h"
#include "tot_state.h"

#include <cstdint>

namespace mod {

class Mod {
public:
	Mod();
    
    // Sets up necessary hooks for the mod's code to run.
    void Init();
    // Code that runs every frame.
    void Update();
    // Code that runs drawing-related code every frame.
    void Draw();
    
    // Holds state specific to the Tower of Trials mod.
    tot::StateManager state_;
    // Holds state specific to the Infinite Pit mod.
    infinite_pit::StateManager_v2 inf_state_;
};

extern Mod* g_Mod;

}