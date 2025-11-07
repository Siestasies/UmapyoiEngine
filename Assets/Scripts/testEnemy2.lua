--basically include
local StateMachine = require("StateMachine")
local WalkState = require("WalkState")

ExposedVars = {
    speed = 100.0,
    name = "unknown",
    isActive = true
}
--reference to FSM
local fsm = nil

local playerEntity = -1

local isWalk = false;

function Start()
    -- Global utilities (same for everyone)
    Log("Script started at " .. GetDeltaTime())
    
    -- Entity-specific context
    Log("My entity ID: " .. EntityID)

    Log("My Name is: " .. name);

    thisEntity = GetEntity(EntityID)

    fsm = StateMachine:new(thisEntity)
    --add states here
    fsm:addState("WalkState2", WalkState)
    --add whatever state else
    -- Set initial state
    fsm:changeState("WalkState2")
    
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
end

function OnCollisionEnter(otherEntity)
    Log(name .. " -- Collision entered -- " .. otherEntity)
    end
end

function OnCollision(otherEntity)
    Log(name .. " -- Collided -- " .. otherEntity)
end

function OnCollisionExit(otherEntity)
    Log(name .. " -- Collided exit -- " .. otherEntity)
end