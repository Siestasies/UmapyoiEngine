--! @file IdleState.lua
--! @par    Project: GAM200
--! @par    Course: CSD2401
--! @par    Section A
--! @par    Software Engineering Project 3

--! @author Koh Kai Yang (100%)
--! @par    E-mail: k.kaiyang@digipen.edu
--! @par    DigiPen login: k.kaiyang

--! @brief State for idle behavior (no movement)
--! @details Entity remains stationary. Listens for input to transition to walk or chase.

--! All content (C) 2025 DigiPen Institute of Technology Singapore.
--! All rights reserved.

local BaseState = require("baseState")
local Vec2 = require("Vec2")

--! @class IdleState
--! @brief Passive idle behavior with state transitions
local IdleState = {}
setmetatable(IdleState, {__index = BaseState})
IdleState.__index = IdleState


--! @brief Constructor for idle state
--! @param fsm StateMachine Reference to the state machine
--! @param parent table The parent entity
--! @return IdleState A new idle state instance
function IdleState:new(fsm, parent)
    local instance = BaseState.new(self, fsm, parent)
    return instance
end


--! @brief Called when entering idle state
--! @details Logs entry message for debugging
function IdleState:enter()
    Log("entering idle")
end


--! @brief Updates idle behavior each frame
--! @details Listens for input to transition to walk or chase states
--! @param dt number Delta time since last frame
function IdleState:update(dt)
    if KeyPressed(KEY_M) then
        self.fsm:changeState("WalkState")
    end
    if KeyPressed(KEY_B) then
        self.fsm:changeState("ChaseState")
    end
end


--! @brief Called when exiting idle state
function IdleState:exit()
end

return IdleState
