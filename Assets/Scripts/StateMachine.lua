--[[!
@file   StateMachine.lua
@par    Project: GAM200
@par    Course: CSD2401
@par    Section A
@par    Software Engineering Project 3

@author Koh Kai Yang (100%)
@par    E-mail: k.kaiyang@digipen.edu
@par    DigiPen login: k.kaiyang

@brief
This is the implementation of state machine for entity behaviour. This handles the add and changing of states for the entity

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
]]

local StateMachine = {}
StateMachine.__index = StateMachine

--[[!
    @brief constructor for the state machine
    @param parent - takes in the script of the entity that holds the states in order to use exposed function from c++
    @return instance of this state machine
]]
-- Constructor now accepts the parent entity object
function StateMachine:new(parent)
    local instance = {
        currentState = nil,
        states = {},
        parent = parent,  -- Store reference to parent entity
        entityId = parent and parent.id or nil  -- Optional: keep ID if parent has one
    }
    setmetatable(instance, self)
    return instance
end

--[[!
    @brief adds the state to the state machine of the entity
    @param name - the name of the state so you can later use to change the state
    @param stateClass - the object of the state to be added to the state array
]]
-- Create state instances with both FSM and parent references
function StateMachine:addState(name, stateClass)
    -- Pass both self (the FSM) and the parent entity to the state
    local state = stateClass:new(self, self.parent)
    self.states[name] = state
end


--[[!
    @brief changes the state
    @param newStateName - the name of the state so you can later use to change the state
]]
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

--[[!
    @brief runs the update functions in the states
    @param dt - delta time to update thing in the states
]]
function StateMachine:update(dt)
    if self.currentState then
        self.currentState:update(dt)
    end
end

--[[!
    @brief returns the name of the current state running
    @return returns the name of the state
]]
function StateMachine:getCurrentStateName()
    for name, state in pairs(self.states) do
        if state == self.currentState then
            return name
        end
    end
    return "None"
end

-- Optional: Helper to get parent entity
function StateMachine:getParent()
    return self.parent
end

return StateMachine
