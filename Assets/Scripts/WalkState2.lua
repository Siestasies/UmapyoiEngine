--! @file WalkState2.lua
--! @par    Project: GAM200
--! @par    Course: CSD2401
--! @par    Section A
--! @par    Software Engineering Project 3

--! @author Koh Kai Yang (100%)
--! @par    E-mail: k.kaiyang@digipen.edu
--! @par    DigiPen login: k.kaiyang

--! @brief State for circular patrol behavior
--! @details Entity moves in a circular path around its starting position.


--! All content (C) 2025 DigiPen Institute of Technology Singapore.
--! All rights reserved.


local BaseState = require("baseState")
local Vec2 = require("Vec2")


--! @class CircleState
--! @brief Circular patrol behavior
local WalkState2 = {}
setmetatable(WalkState2, {__index = BaseState})
WalkState2.__index = WalkState2



--! @brief Constructor for circle state
--! @param fsm StateMachine Reference to the state machine
--! @param parent table The parent entity with transform and rigidbody
--! @return WalkState2 A new circle state instance
function WalkState2:new(fsm, parent)
    local instance = BaseState.new(WalkState2, fsm, parent)
    
    instance.radius = 10.0           -- Circle radius
    instance.angularSpeed = 1.0       -- Speed of rotation (radians per second)
    instance.angle = 0                -- Current angle
    instance.centerPos = Vec2.new(0, 0)  -- Center of circle
    instance.speed = 300              -- Movement speed for physics
    instance.currentAccel = Vec2.new(0, 0)
    instance.accelSmoothFactor = 15.0
    
    return instance
end



--! @brief Called when entering circle state
--! @details Stores the starting position as circle center
function WalkState2:enter()
    Log("entering circle state")
    if self.parent:HasTransform() then
        local transform = self.parent:GetTransform()
        self.centerPos = Vec2.new(transform.position.x, transform.position.y)
        self.angle = 0
    end
end



--! @brief Called when exiting circle state
--! @details Clears acceleration to stop movement
function WalkState2:exit()
    Log("leaving circle state")
    if self.parent:HasRigidBody() then
        local rb = self.parent:GetRigidBody()
        rb.acceleration.x = 0
        rb.acceleration.y = 0
    end
end



--! @brief Updates circular movement each frame
--! @details Calculates target position on circle and moves entity toward it
--! @param dt number Delta time since last frame
function WalkState2:update(dt)
    if not self.parent or not self.parent.isValid then
        return
    end
    
    if self.parent:HasTransform() and self.parent:HasRigidBody() then
        -- Increment angle based on angular speed
        self.angle = self.angle + self.angularSpeed * dt
        
        -- Calculate target position on circle using parametric equations
        local targetX = self.centerPos.x + self.radius * math.cos(self.angle)
        local targetY = self.centerPos.y + self.radius * math.sin(self.angle)
        local targetPos = Vec2.new(targetX, targetY)
        
        -- Get current position
        local transform = self.parent:GetTransform()
        local currPos = Vec2.new(transform.position.x, transform.position.y)
        
        -- Calculate direction to target
        local direction = targetPos - currPos
        local distance = direction:length()
        
        if distance > 0.1 then
            direction = direction:normalize()
        else
            direction = Vec2.new(0, 0)
        end
        
        -- Apply acceleration toward target position
        local rb = self.parent:GetRigidBody()
        local targetAccel = direction * self.speed
        
        self.currentAccel = self.currentAccel + (targetAccel - self.currentAccel) * self.accelSmoothFactor * dt
        rb.acceleration.x = self.currentAccel.x
        rb.acceleration.y = self.currentAccel.y
    end
end


return WalkState2
