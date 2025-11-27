--! @file   ChaseState.lua
--! @par    Project: GAM200
--! @par    Course: CSD2401
--! @par    Section A
--! @par    Software Engineering Project 3

--! @author Koh Kai Yang (100%)
--! @par    E-mail: k.kaiyang@digipen.edu
--! @par    DigiPen login: k.kaiyang

--! @brief State for pursuing a player entity
--! @details Smoothly accelerates toward the player's position using Vec2 math.
--! Transitions to WalkState (M key) or IdleState (N key).

--! All content (C) 2025 DigiPen Institute of Technology Singapore.
--! All rights reserved.

local BaseState = require("baseState")
local Vec2 = require("Vec2")

--! @class ChaseState
--! @brief Player pursuit behavior with smooth acceleration
local ChaseState = {}
setmetatable(ChaseState, {__index = BaseState})
ChaseState.__index = ChaseState


--! @brief Constructor for chase state
--! @param fsm StateMachine Reference to the state machine
--! @param parent table The parent entity with transform and rigidbody
--! @return ChaseState A new chase state instance
function ChaseState:new(fsm, parent)
    local instance = BaseState.new(self, fsm, parent)
    
    instance.playerPos = Vec2.new(0, 0)
    instance.speed = 10
    instance.currentAccel = Vec2.new(0, 0)
    instance.accelSmoothFactor = 15.0

    return instance
end


--! @brief Called when entering chase state
--! @details Logs entry message for debugging
function ChaseState:enter()
    Log("entered the chase state")
    -- local transform = self.parent:GetTransform()
    -- local playerTransform = self.parent.playerEntity:GetTransform()

    -- --insert pathfinding
    -- -- local pf = self.parent:GetPathFinding()
    -- -- pf.goal = playerTransform
    -- SetPathFindingGoal(playerTransform.position)

    -- local distance = Vec2.new(transform.position.x - playerTransform.position.x,transform.position.y - playerTransform.position.y)
    -- if distance:length() > 5 then
    --     self.fsm:changeState("IdleState")
    -- end
end


--! @brief Called when exiting chase state
--! @details Clears acceleration to stop movement
function ChaseState:exit()
    Log("leaving the chase state")
    if self.parent:HasRigidBody() then
        rb = self.parent:GetRigidBody()
        rb.acceleration.x = 0
        rb.acceleration.y = 0
    end

    local transform = self.parent:GetTransform()

    --when leaving state make sure that the enemy stops moving so im setting the goal to current position
    SetPathFindingGoal(self.parent.id,transform.position.x,transform.position.y)
end


--! @brief Updates chase behavior each frame
--! @details Finds player position, calculates movement vector, applies smoothed acceleration
--! @param dt number Delta time since last frame
function ChaseState:update(dt)
    -- if not self.parent or not self.parent.isValid then
    --     return
    -- end
    -- 
    -- playerEntity = FindEntityWithComponent("Player")
    -- player = GetEntity(playerEntity)
    -- if playerEntity ~= -1 then
    --     local playerTf = player:GetTransform()
    --     if playerTf then
    --         self.playerPos.x = playerTf.position.x
    --         self.playerPos.y = playerTf.position.y
    --     end
    -- end
    -- 
    -- if self.parent:HasTransform() and self.parent:HasRigidBody() then
    --     local rb = self.parent:GetRigidBody()
    --     local pos = Vec2.new(self.parent:GetTransform().position.x , self.parent:GetTransform().position.y)
    --     local moveVec = Vec2.new(self.playerPos.x - pos.x , self.playerPos.y - pos.y)
    --     
    --     if moveVec.x ~= 0 or moveVec.y ~= 0 then
    --         local targetAccel = moveVec * self.speed
    --         self.currentAccel = self.currentAccel + (targetAccel - self.currentAccel) * self.accelSmoothFactor * dt
    --         rb.acceleration.x = self.currentAccel.x
    --         rb.acceleration.y = self.currentAccel.y
    --     else
    --         self.currentAccel = self.currentAccel + (Vec2.new(0, 0) - self.currentAccel) * self.accelSmoothFactor * dt
    --         rb.acceleration = self.currentAccel
    --     end
    -- end
    -- 
    -- if KeyPressed(KEY_1) then
    --     self.fsm:changeState("WalkState")
    -- end
    -- if KeyPressed(KEY_2) then
    --     self.fsm:changeState("IdleState")
    -- end

    local transform = self.parent:GetTransform()
    local playerTransform = GetTransformFrom(self.parent.playerEntity)

    --insert pathfinding
    -- local pf = self.parent:GetPathFinding()
    -- pf.goal = playerTransform
    SetPathFindingGoal(self.parent.id,playerTransform.position.x,playerTransform.position.y)

    local distance = Vec2.new(transform.position.x - playerTransform.position.x,transform.position.y - playerTransform.position.y)
    if distance:length() > 75 then
        self.fsm:changeState("IdleState")
    end
end

return ChaseState
