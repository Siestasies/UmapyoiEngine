--! @file   StateMachine.lua
--! @par    Project: GAM200
--! @par    Course: CSD2401
--! @par    Section A
--! @par    Software Engineering Project 3

--! @author Koh Kai Yang (100%)
--! @par    E-mail: k.kaiyang@digipen.edu
--! @par    DigiPen login: k.kaiyang

--! @brief Finite state machine for managing entity behavior states
--! @details Handles state transitions, updates, and lifecycle (enter/exit) for entities
--! with access to parent entity functions and C++ bindings

--! All content (C) 2025 DigiPen Institute of Technology Singapore.
--! All rights reserved.

--! @class StateMachine
--! @brief Manages state transitions and updates for entity behavior systems
local StateMachine = {}
StateMachine.__index = StateMachine


--! @brief Constructor for a new state machine instance
--! @param parent table The parent entity object containing id and C++ exposed functions
--! @return StateMachine A new state machine instance with empty states table
function StateMachine:new(parent)
    local instance = {
        currentState = nil,
        states = {},
        parent = parent,
        entityId = parent and parent.id or nil
    }
    setmetatable(instance, self)
    return instance
end


--! @brief Adds a state to the state machine
--! @details Instantiates the state class with references to the FSM and parent entity
--! @param name string The unique identifier for this state (used in changeState calls)
--! @param stateClass table The state class with a :new(fsm, parent) constructor
function StateMachine:addState(name, stateClass)
    local state = stateClass:new(self, self.parent)
    self.states[name] = state
end


--! @brief Transitions to a different state
--! @details Calls exit() on the current state, then enter() on the new state
--! @param newStateName string The name of the state to transition to
--! @throws error If the specified state name does not exist in the states table
function StateMachine:changeState(newStateName)
    local newState = self.states[newStateName]
    if not newState then
        LogError("State '" .. newStateName .. "' not found!")
        return
    end
    
    if self.currentState then
        self.currentState:exit()
    end
    
    self.currentState = newState
    self.currentState:enter()
end


--! @brief Updates the current active state
--! @details Passes delta time to the current state's update method
--! @param dt number Delta time since last frame (in seconds)
function StateMachine:update(dt)
    if self.currentState then
        self.currentState:update(dt)
    end
end


--! @brief Retrieves the name of the currently active state
--! @return string The name of the current state, or "None" if no state is active
function StateMachine:getCurrentStateName()
    for name, state in pairs(self.states) do
        if state == self.currentState then
            return name
        end
    end
    return "None"
end


--! @brief Retrieves the parent entity object
--! @return table The parent entity that owns this state machine
function StateMachine:getParent()
    return self.parent
end


return StateMachine