--imports base class interfaces
local BaseState = require("baseState")
local Vec2 = require("Vec2")

local ChaseState = {}
setmetatable(ChaseState, {__index = BaseState})  -- inherit from BaseState
ChaseState.__index = ChaseState --for instances to fine walk state

--declare new state which inherits from BaseState
function ChaseState:new(fsm, parent)
    -- Call parent constructor to set up basic state properties
    local instance = BaseState.new(self, fsm, parent)
    -- Add your state-specific variables here
    
    instance.playerPos = Vec2.new(0,0)
    instance.speed = 10
    instance.currentAccel = Vec2.new(0, 0)
    instance.accelSmoothFactor = 15.0

    return instance
end

function ChaseState:enter()
    Log("entered the chase state")
end

function ChaseState:exit()
    Log("leaving the chase state")
    if self.parent:HasRigidBody() then
        rb = self.parent:GetRigidBody()
        rb.acceleration.x = 0
        rb.acceleration.y = 0
    end
end

function ChaseState:update(dt)
    if not self.parent or not self.parent.isValid then
        return
    end

    playerEntity = FindEntityWithComponent("Player")
    player = GetEntity(playerEntity)
    if playerEntity ~= -1 then
        local playerTf = player:GetTransform()
        if playerTf then
            self.playerPos.x = playerTf.position.x
            self.playerPos.y = playerTf.position.y
        end
    end
    
    if self.parent:HasTransform() and self.parent:HasRigidBody() then
        local rb = self.parent:GetRigidBody()
        local pos = Vec2.new(self.parent:GetTransform().position.x , self.parent:GetTransform().position.y)
        local moveVec = Vec2.new(self.playerPos.x - pos.x ,self.playerPos.y - pos.y)
        
        if moveVec.x ~= 0 or moveVec.y ~= 0 then
            local targetAccel = moveVec * self.speed
            self.currentAccel = self.currentAccel + (targetAccel - self.currentAccel) * self.accelSmoothFactor * dt
            rb.acceleration.x = self.currentAccel.x
            rb.acceleration.y = self.currentAccel.y
        else
            self.currentAccel = self.currentAccel + (Vec2.new(0, 0) - self.currentAccel) * self.accelSmoothFactor * dt
            rb.acceleration = self.currentAccel
        end
    end
    
    if KeyPressed(KEY_M) then
        self.fsm:changeState("WalkState")
    end
    if KeyPressed(KEY_N) then
        self.fsm:changeState("IdleState")
    end
end

return ChaseState