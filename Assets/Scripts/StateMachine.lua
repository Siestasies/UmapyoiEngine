local StateMachine = {}
StateMachine.__index = StateMachine

function StateMachine:new()
    local instance = {
        currentState = nil,
        states = {},
        entityId = EntityID  -- Use global EntityID from API
    }
    setmetatable(instance, self)
    return instance
end

function StateMachine:addState(name, stateClass)
    -- Create new instance of state for this FSM
    local state = stateClass:new(self)
    self.states[name] = state
end

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

function StateMachine:update(dt)
    if self.currentState then
        self.currentState:update(dt)
    end
end

function StateMachine:getCurrentStateName()
    for name, state in pairs(self.states) do
        if state == self.currentState then
            return name
        end
    end
    return "None"
end

return StateMachine
