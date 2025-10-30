-- Make engine functions available to modules
_G.HasTransform = HasTransform
_G.HasRigidBody = HasRigidBody
_G.GetRigidBody = GetRigidBody
_G.GetTransform = GetTransform
_G.Vec2 = Vec2
_G.KeyPressed = KeyPressed
_G.KEY_U = KEY_U
_G.Log = Log
_G.LogError = LogError

--basically include
local StateMachine = require("StateMachine")
local WalkState = require("WalkState")
local TestState = require("TestState")

ExposedVars = {
    speed = 100.0,
    name = "unknown",
    isActive = true
}
--reference to FSM
local fsm = nil

local playerEntity = -1

function Start()
    -- Global utilities (same for everyone)
    Log("Script started at " .. GetDeltaTime())
    
    -- Entity-specific context
    Log("My entity ID: " .. EntityID)

    Log("My Name is: " .. name);

    local capturedId = EntityID
    fsm = StateMachine:new(capturedId)
    --add states here
    fsm:addState("WalkState", WalkState)
    fsm:addState("TestState",TestState)
    --add whatever state else
    -- Set initial state
    fsm:changeState("WalkState")
    
    local myTransform = GetTransform(EntityID)
    if myTransform then
        Log("My position: " .. myTransform.position.x .. ", " .. myTransform.position.y)
    end
    
    -- Cross-entity access
    playerEntity = FindEntityWithComponent("Player")
    Log("player : " .. playerEntity)
    if playerEntity ~= -1 then
        local playerTf = GetTransformFrom(playerEntity)
        if playerTf then
            Log("Player position: " .. playerTf.position.x .. ", " .. playerTf.position.y)
        end
    end
end

function Update(dt)
    local transform = GetTransform(EntityID)
    if fsm then
        fsm:update(dt)
    end

    --local rb = GetRigidBody()
    if transform then 
        --rb.velocity.x = -200 * speed * dt
        --Log(" position: " .. transform.position.x .. ", " .. transform.position.y)
        
        --local playerTf = GetTransformFrom(playerEntity)
        --Log("Player is at: " .. playerTf.position.x .. ", " .. playerTf.position.y)
        --if playerEntity ~= -1 then
        --    local playerTf = GetTransform(playerEntity)
        --    if playerTf then
        --        -- Calculate direction to player
        --        local dx = playerTf.position.x - transform.position.x
        --        local dy = playerTf.position.y - transform.position.y
        --        
        --        Log("Player is at: " .. playerTf.position.x .. ", " .. playerTf.position.y)
        --    end
        --end

    else
        Log("components are missing");
    end

    --if KeyPressed(KEY_W) then 
    --    Log("W IS PRESSED")
    --elseif KeyDown(KEY_W) then 
    --    Log("W IS DOWN")
    --elseif KeyReleased(KEY_W) then 
    --    Log("W IS RELEASE")
    --end
    
    --transform.scale = 1.1 * transform.scale
    
    --Log("Enemy position: " .. transform.position.x)
end

function OnCollisionEnter(otherEntity)
    Log(name .. " -- Collision entered -- " .. otherEntity)
end

function OnCollision(otherEntity)
    Log(name .. " -- Collided -- " .. otherEntity)
end

function OnCollisionExit(otherEntity)
    Log(name .. " -- Collided exit -- " .. otherEntity)
end