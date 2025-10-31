-- base_state.lua
local BaseState = {} --its like the declaration of a class in c++

--this provides a reference back to base class in case an implementation is missing in the child
BaseState.__index = BaseState 

--this is basically a constructor for the inheritance 
function BaseState:new(fsm, parent)
    local instance = {
        fsm = fsm,
        parent = parent
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

--Helper to safely access parent
function BaseState:getParent()
    return self.parent
end

return BaseState

--[[
HOW TO USE: Template State Class

This template provides a starting point for creating individual states in your FSM.
Each state can access:
  - self.fsm: Reference to the finite state machine
  - self.parent: Reference to the parent entity (e.g., enemy, player, NPC)
  
Usage Example:
  local entity = { health = 100, position = {x=0, y=0} }
  local fsm = FSM:new(entity)
  local myState = MyState:new(fsm, entity)
  
  -- Import the base state class
  local BaseState = require("baseState")
  
  -- Create the state class
  local EmptyState = {}
  
  -- Set up inheritance from BaseState
  setmetatable(EmptyState, {__index = BaseState})
  EmptyState.__index = EmptyState
  
  -- Constructor - Creates a new instance of this state
  function EmptyState:new(fsm, parent)
    -- Call parent constructor to set up basic state properties
    local instance = BaseState.new(self, fsm, parent)
    
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
    -- Cleanup state-specific logic here
    -- Example: Reset parent properties
    -- if self.parent then
    --     self.parent.isAnimating = false
    -- end
end

-- Return the state class so require() works
return EmptyState
]]
