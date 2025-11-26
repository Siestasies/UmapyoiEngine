--! @file WalkState.lua
--! @par    Project: GAM200
--! @par    Course: CSD2401
--! @par    Section A
--! @par    Software Engineering Project 3

--! @author Koh Kai Yang (100%)
--! @par    E-mail: k.kaiyang@digipen.edu
--! @par    DigiPen login: k.kaiyang

--! @brief State for patrol behavior along a fixed path
--! @details Entity walks in one direction, reverses at distance threshold.
--! Transitions to IdleState (N key) or ChaseState (B key).

--! All content (C) 2025 DigiPen Institute of Technology Singapore.
--! All rights reserved.

local BaseState = require("baseState")
local Vec2 = require("Vec2")

--! @class WalkState
--! @brief Patrol behavior with directional reversal
local WalkState = {}
setmetatable(WalkState, {__index = BaseState})
WalkState.__index = WalkState


--! @brief Constructor for walk state
--! @param fsm StateMachine Reference to the state machine
--! @param parent table The parent entity with transform and rigidbody
--! @return WalkState A new walk state instance
function WalkState:new(fsm, parent)
    local instance = BaseState.new(WalkState, fsm, parent)
    
    instance.speed = 100
    instance.currentAccel = Vec2.new(0, 0)
    instance.accelSmoothFactor = 15.0
    instance.startPos = Vec2.new(0, 0)
    instance.moveVec = Vec2.new(1, 0)
    
    return instance
end


--! @brief Called when entering walk state
--! @details Initializes patrol starting position
function WalkState:enter()
    Log("entering walk state")
    if self.parent:HasTransform() then
        startPos = Vec2.new(self.parent:GetTransform().position.x, self.parent:GetTransform().position.y)
    end
end


--! @brief Called when exiting walk state
--! @details Clears acceleration to stop movement
function WalkState:exit()
    Log("leaving walk")
    if self.parent:HasRigidBody() then
        rb = self.parent:GetRigidBody()
        rb.acceleration.x = 0
        rb.acceleration.y = 0
    end
end


--! @brief Updates walk behavior each frame
--! @details Moves in patrol direction, reverses when distance threshold exceeded
--! @param dt number Delta time since last frame
function WalkState:update(dt)
    if not self.parent or not self.parent.isValid then
        return
    end

    if self.parent.mHealth <= 0 then
        self.fsm:changeState("DieState")
    end
    
    if self.parent:HasTransform() and self.parent:HasRigidBody() then
        currPos = Vec2.new(self.parent:GetTransform().position.x , self.parent:GetTransform().position.y)
        
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
            local targetAccel = self.moveVec * self.speed
            self.currentAccel = self.currentAccel + (targetAccel - self.currentAccel) * self.accelSmoothFactor * dt
            rb.acceleration.x = self.currentAccel.x
            rb.acceleration.y = self.currentAccel.y
        else
            self.currentAccel = self.currentAccel + (Vec2.new(0, 0) - self.currentAccel) * self.accelSmoothFactor * dt
            rb.acceleration = self.currentAccel
        end
    end
    
    -- if KeyPressed(KEY_2) then
    --     Log("pressed")
    --     self.fsm:changeState("IdleState")
    -- end
    -- if KeyPressed(KEY_3) then
    --     Log("pressed")
    --     self.fsm:changeState("ChaseState")
    -- end
end

return WalkState
