-- base_state.lua
local BaseState = {} --its like the declaration of a class in c++

--this provides a reference back to base class in case an implementation is missing in the child
BaseState.__index = BaseState 

--this is basically a constructor for the inheritance 
function BaseState.new(class, fsm, parent)
    local instance = setmetatable({}, class)  -- Use the class passed in
    instance.fsm = fsm
    instance.parent = parent
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
  local fsm = StateMachine:new(entity)
  
  -- In your main script:
  fsm:addState("MyState", MyState)  -- Pass the class, not an instance
  fsm:changeState("MyState")

---------------------------------template------------------------------------
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
    -- This creates an instance with EmptyState as its metatable
    local instance = BaseState.new(self, fsm, parent)
    
    -- Add your state-specific variables here
    -- instance.speed = 100
    -- instance.myProperty = "value"
    -- instance.counter = 0
    
    return instance
end

-- Called once when entering this state
function EmptyState:enter()
    -- Initialize or reset state-specific logic
    -- Example: Set parent properties
    -- if self.parent then
    --     self.parent.isAnimating = true
    -- end
    
    print("Entered EmptyState")
end

-- Called every frame while in this state
function EmptyState:update(dt)
    -- Update state logic here
    -- Access state properties with self.
    -- Access parent entity with self.parent
    -- Access FSM with self.fsm
    
    -- Example: Check for state transitions
    -- if someCondition then
    --     self.fsm:changeState("OtherState")
    -- end
end

-- Called once when exiting this state
function EmptyState:exit()
    -- Cleanup state-specific logic here
    -- Example: Reset parent properties
    -- if self.parent then
    --     self.parent.isAnimating = false
    -- end
    
    print("Exited EmptyState")
end

-- Return the state class so require() works
return EmptyState
]]
