--! @file   baseState.lua
--! @par    Project: GAM200
--! @par    Course: CSD2401
--! @par    Section A
--! @par    Software Engineering Project 3

--! @author Koh Kai Yang (100%)
--! @par    E-mail: k.kaiyang@digipen.edu
--! @par    DigiPen login: k.kaiyang

--! @brief Base class for implementing entity behavior states
--! @details Provides the interface and common functionality for state inheritance.
--! Derived states should override enter(), exit(), and update() methods.

--! All content (C) 2025 DigiPen Institute of Technology Singapore.
--! All rights reserved.

--! @class BaseState
--! @brief Abstract base class for entity state implementations
--! @details Implements the state pattern with virtual methods and FSM integration.
--! Acts as a foundation for creating specialized state classes.
local BaseState = {}
BaseState.__index = BaseState


--! @brief Constructor for state instantiation with inheritance
--! @details Sets up the state instance with FSM and parent entity references.
--! Uses Lua's metatables to enable inheritance patterns.
--! @param class table The derived class to instantiate
--! @param fsm StateMachine Reference to the state machine managing this state
--! @param parent table The parent entity object with C++ exposed functions
--! @return BaseState A new state instance with inherited methods
function BaseState.new(class, fsm, parent)
    local instance = setmetatable({}, class)
    instance.fsm = fsm
    instance.parent = parent
    return instance
end


--! @brief Virtual method called when entering this state
--! @details Override this method in derived classes to handle state entry logic
function BaseState:enter() end


--! @brief Virtual method called when exiting this state
--! @details Override this method in derived classes to handle state exit cleanup
function BaseState:exit() end


--! @brief Virtual method called each frame to update state logic
--! @details Override this method in derived classes for per-frame updates
--! @param dt number Delta time since last frame (in seconds)
function BaseState:update(dt) end


--! @brief Transitions the FSM to a different state
--! @details Convenience method for state transitions using the FSM reference
--! @param newStateName string The name of the state to transition to
--! @throws error If FSM reference is unavailable (logs error instead of throwing)
function BaseState:changeState(newStateName)
    if self.fsm then
        self.fsm:changeState(newStateName)
    else
        LogError("No FSM reference in state!")
    end
end


--! @brief Checks if this state is currently active
--! @return boolean True if this state is the FSM's current state
function BaseState:isCurrentState()
    return self.fsm and self.fsm:getCurrentStateName() == self
end


--! @brief Retrieves the parent entity object
--! @details Provides safe access to the parent entity and its C++ functions
--! @return table The parent entity that owns this state
function BaseState:getParent()
    return self.parent
end


return BaseState

--reference of how to use the state class

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
