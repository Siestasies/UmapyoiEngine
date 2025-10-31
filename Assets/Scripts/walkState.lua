local BaseState = require("baseState")
local Vec2 = require("Vec2")

local WalkState = {}
setmetatable(WalkState, {__index = BaseState})
WalkState.__index = WalkState

function WalkState:new(fsm, parent)
    --Pass WalkState or self
    local instance = BaseState.new(WalkState, fsm, parent)
    
    instance.speed = 100
    instance.currentAccel = Vec2.new(0, 0)
    instance.accelSmoothFactor = 15.0
    
    return instance
end

function WalkState:enter()
    Log("entering walk state")
end

function WalkState:exit()
    Log("leaving walk")
end

function WalkState:update(dt)
    if not self.parent or not self.parent.isValid then
        return
    end
    
    if self.parent:HasTransform() and self.parent:HasRigidBody() then
        local rb = self.parent:GetRigidBody()
        local moveVec = Vec2.new(1, 0)
        
        if moveVec.x ~= 0 or moveVec.y ~= 0 then
            local targetAccel = moveVec * self.speed  -- FIX: Add self.
            self.currentAccel = self.currentAccel + (targetAccel - self.currentAccel) * self.accelSmoothFactor * dt
            rb.acceleration.x = self.currentAccel.x
            rb.acceleration.y = self.currentAccel.y
        else
            self.currentAccel = self.currentAccel + (Vec2.new(0, 0) - self.currentAccel) * self.accelSmoothFactor * dt
            rb.acceleration = self.currentAccel
        end
    end
    
    if KeyPressed(KEY_M) then
        Log("pressed")
        self.fsm:changeState("IdleState")
    end
end

return WalkState
