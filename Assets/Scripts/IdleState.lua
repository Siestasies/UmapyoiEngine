-- Import the base state class
  local BaseState = require("baseState")
  local Vec2 = require("Vec2")
  
  -- Create the state class
  local IdleState = {}
  
  -- Set up inheritance from BaseState
  setmetatable(IdleState, {__index = BaseState})
  IdleState.__index = IdleState
  
  -- Constructor - Creates a new instance of this state
  function IdleState:new(fsm, parent)
    -- Call parent constructor to set up basic state properties
    local instance = BaseState.new(self, fsm, parent)
    -- Add your state-specific variables here
    
    return instance
end

-- Called once when entering this state
function IdleState:enter()
    Log("entering idle")
    if self.parent:HasRigidBody() then
        rb = self.parent:GetRigidBody()
        rb.acceleration.x = 0
        rb.acceleration.y = 0
    end
end

-- Called every frame while in this state
function IdleState:update(dt)
    if KeyPressed(KEY_N) then
        self.fsm:changeState("WalkState")
    end
    if KeyPressed(KEY_B) then
        self.fsm:changeState("ChaseState")
    end
end

-- Called once when exiting this state
function IdleState:exit()
end

-- Return the state class so require() works
return IdleState