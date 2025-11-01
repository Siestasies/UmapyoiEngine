local StateMachine = {}
StateMachine.__index = StateMachine

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

-- Create state instances with both FSM and parent references
function StateMachine:addState(name, stateClass)
    -- Pass both self (the FSM) and the parent entity to the state
    local state = stateClass:new(self, self.parent)
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

-- Optional: Helper to get parent entity
function StateMachine:getParent()
    return self.parent
end

return StateMachine
