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
    instance.startPos = Vec2.new(0, 0)
    instance.moveVec = Vec2.new(1, 0)
    
    return instance
end

function WalkState:enter()
    Log("entering walk state")
    if self.parent:HasTransform() then
        startPos = Vec2.new(self.parent:GetTransform().position.x, self.parent:GetTransform().position.y)
    end
end

function WalkState:exit()
    Log("leaving walk")
    if self.parent:HasRigidBody() then
        rb = self.parent:GetRigidBody()
        rb.acceleration.x = 0
        rb.acceleration.y = 0
    end
end

function WalkState:update(dt)
    if not self.parent or not self.parent.isValid then
        return
    end
    
    if self.parent:HasTransform() and self.parent:HasRigidBody() then

        currPos = Vec2.new(self.parent:GetTransform().position.x , self.parent:GetTransform().position.y)
        --Log("distance" .. currPos:distance(self.startPos))
        if currPos:distance(self.startPos) > 100 then
            self.startPos.x = currPos.x
            self.startPos.y = currPos.y
            if self.moveVec.x == 1 then
                self.moveVec.x = -1
            elseif self.moveVec.x == -1 then
                self.moveVec.x = 1
            end
            Log("move vec" .. self.moveVec.x)
        end

        local rb = self.parent:GetRigidBody()

        if self.moveVec.x ~= 0 or self.moveVec.y ~= 0 then
            local targetAccel = self.moveVec * self.speed  -- FIX: Add self.
            self.currentAccel = self.currentAccel + (targetAccel - self.currentAccel) * self.accelSmoothFactor * dt
            rb.acceleration.x = self.currentAccel.x
            rb.acceleration.y = self.currentAccel.y
        else
            self.currentAccel = self.currentAccel + (Vec2.new(0, 0) - self.currentAccel) * self.accelSmoothFactor * dt
            rb.acceleration = self.currentAccel
        end
    end
    
    if KeyPressed(KEY_N) then
        Log("pressed")
        self.fsm:changeState("IdleState")
    end
    if KeyPressed(KEY_B) then
        Log("pressed")
        self.fsm:changeState("ChaseState")
    end
end

return WalkState
