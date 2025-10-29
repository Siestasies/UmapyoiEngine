-- base_state.lua
local BaseState = {} --its like the declaration of a class in c++

--this provides a reference back to base class in case an implementation is missing in the child
BaseState.__index = BaseState 

--this is basically a constructor for the inheritance 
function BaseState:new(fsm)
    local instance = {
        fsm = fsm
    }
    setmetatable(instance, self)
    return instance
end

-- Default implementations (like your virtual functions)
function BaseState:enter() end
function BaseState:exit() end
function BaseState:update(dt) end

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

--[[
how to use? template state class
-- Import the base state class
local BaseState = require("baseState")

-- Create the state class
local EmptyState = {}

-- Set up inheritance from BaseState
setmetatable(EmptyState, {__index = BaseState})
EmptyState.__index = EmptyState

-- Constructor - Creates a new instance of this state
function EmptyState:new(fsm)
    -- Call parent constructor to set up basic state properties
    local instance = BaseState.new(self, fsm)
    
    -- Add your state-specific variables here
    
    return instance
end

-- Called once when entering this state
function EmptyState:enter()
    
end

-- Called every frame while in this state
function EmptyState:update(dt)
    
end

-- Called once when exiting this state
function EmptyState:exit()
    
end

--Return the state class so require() works
return EmptyState
]]
