ExposedVars = {
    speed = 100.0,
    name = "unknown",
    isActive = true
}

local playerEntity = -1

function Start()
    -- Global utilities (same for everyone)
    Log("Script started at " .. GetDeltaTime())
    
    -- Entity-specific context
    Log("My entity ID: " .. EntityID)

    Log("My Name is: " .. name);
    
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

    --set initial state
    ChangeState(entityID, "walk")
end

function Update(dt)
    local transform = GetTransform(EntityID)

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