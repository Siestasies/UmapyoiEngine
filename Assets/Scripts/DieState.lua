
local BaseState = require("baseState")
local Vec2 = require("Vec2")

--! @class IdleState
--! @brief Passive idle behavior with state transitions
local DieState = {}
setmetatable(DieState, {__index = BaseState})
DieState.__index = DieState


--! @brief Constructor for idle state
--! @param fsm StateMachine Reference to the state machine
--! @param parent table The parent entity
--! @return IdleState A new idle state instance
function DieState:new(fsm, parent)
    local instance = BaseState.new(self, fsm, parent)
    return instance
end


--! @brief Called when entering idle state
--! @details Logs entry message for debugging
function DieState:enter()
    Log("entering die")
end


--! @brief Updates idle behavior each frame
--! @details Listens for input to transition to walk or chase states
--! @param dt number Delta time since last frame
function DieState:update(dt)
    Log("im dying ere")
    DestroyWithChildren(self.parent.id)
end


--! @brief Called when exiting idle state
function DieState:exit()
    Log("exiting die")
end

return DieState
