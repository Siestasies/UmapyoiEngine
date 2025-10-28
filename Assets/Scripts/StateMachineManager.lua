-- state_machine.lua
local StateMachine = {}
StateMachine.__index = StateMachine

function StateMachine.new(initialStateName)
    local sm = {
        states = {},           -- Table of state name -> state instance
        currentState = nil,
        currentStateName = initialStateName or "idle",
        previousStateName = nil
    }
    setmetatable(sm, StateMachine)
    return sm
end

-- Register a state class (not instance)
function StateMachine:addState(stateName, stateClass)
    -- Create instance and pass self as FSM reference
    self.states[stateName] = stateClass:new(self)
    
    -- Enter initial state if this is the first state added
    if self.currentStateName == stateName and not self.currentState then
        self.currentState = self.states[stateName]
        if self.currentState.enter then
            self.currentState:enter()
        end
    end
end

-- Change to a different state
function StateMachine:changeState(newStateName)
    if not self.states[newStateName] then
        LogError("State '" .. newStateName .. "' does not exist!")
        return false
    end
    
    -- Exit current state
    if self.currentState and self.currentState.exit then
        self.currentState:exit()
    end
    
    -- Change state
    self.previousStateName = self.currentStateName
    self.currentStateName = newStateName
    self.currentState = self.states[newStateName]
    
    -- Enter new state
    if self.currentState.enter then
        self.currentState:enter()
    end
    
    Log("State changed: " .. (self.previousStateName or "none") .. " -> " .. self.currentStateName)
    return true
end

-- Update current state
function StateMachine:update(dt)
    if self.currentState and self.currentState.update then
        self.currentState:update(dt)
    end
end

function StateMachine:isInState(stateName)
    return self.currentStateName == stateName
end

function StateMachine:getCurrentState()
    return self.currentState
end

function StateMachine:getCurrentStateName()
    return self.currentStateName
end

function StateMachine:getPreviousStateName()
    return self.previousStateName
end

return StateMachine
