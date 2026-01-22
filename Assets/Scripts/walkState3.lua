--! @file WalkState.lua
--! @par    Project: GAM200
--! @par    Course: CSD2401
--! @par    Section A
--! @par    Software Engineering Project 3

--! @author Koh Kai Yang (100%)
--! @par    E-mail: k.kaiyang@digipen.edu
--! @par    DigiPen login: k.kaiyang

--! @brief State for patrol behavior along a fixed path
--! @details Entity walks in one direction, reverses at distance threshold.
--! Transitions to IdleState (N key) or ChaseState (B key).

--! All content (C) 2025 DigiPen Institute of Technology Singapore.
--! All rights reserved.

local Vec2 = require("Vec2")

--! @class WalkState
--! @brief Patrol behavior with directional reversal

--! @brief Called when entering walk state
--! @details Initializes patrol starting position
function state_enter(entity)
    Log("entering walk state")
    
end


--! @brief Called when exiting walk state
--! @details Clears acceleration to stop movement
function state_exit(entity)
    Log("leaving walk")
end


--! @brief Updates walk behavior each frame
--! @details Moves in patrol direction, reverses when distance threshold exceeded
--! @param dt number Delta time since last frame
function state_update(dt)

end

