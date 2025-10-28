-- base_state.lua
local BaseState = {} --its like the declaration of a class in c++

--this provides a reference back to base class in case an implementation is missing in the child
BaseState.__index = BaseState 

--this is basically a constructor for the inheritance 
function BaseState:new()
    local instance = {
        fsm = fsm --reference to the state machine
    }
    setmetatable(instance, self)
    return instance
end

-- Default implementations (like your virtual functions)
function BaseState:enter()

end

function BaseState:exit()

end

function BaseState:update(dt)
    -- Abstract method - must be overridden
end

-- Helper method to change states
function BaseState:changeState(newStateName)
    if self.fsm then
        self.fsm:changeState(newStateName)
    else
        LogError("No FSM reference in state!")
    end
end

-- Helper to check current state
function BaseState:isCurrentState()
    return self.fsm and self.fsm:getCurrentState() == self
end

return BaseState

/*
how to use?
--imports base class interfaces
local BaseState = require("base_state")

--declare new state which inherits from BaseState
local newState = BaseState:new()
function newState:enter()

end

function newState:exit()

end

function newState:update(dt)
    insert implementation here
end

return newState

*/
