ExposedVars = {
    speed = 100.0,
    name = "unknown",
    isActive = true
}

function Start()
    -- Global utilities (same for everyone)
    Log("Script started at " .. GetDeltaTime())
    
    -- Entity-specific context
    Log("My entity ID: " .. EntityID)

    Log("My Name is: " .. name);
    
    local myTransform = GetTransform()
    if myTransform then
        Log("My position: " .. myTransform.position.x .. ", " .. myTransform.position.y)
    end
    
    -- Cross-entity access
    local player = FindEntityWithComponent("Player")
    if player ~= -1 then
        local playerTf = GetTransformFrom(player)
        if playerTf then
            Log("Player position: " .. playerTf.position.x .. ", " .. playerTf.position.y)
        end
    end
end

function Update(dt)
    local transform = GetTransform()
    local rb = GetRigidBody()

    if transform and rb then 
        --rb.velocity.x = -200 * speed * dt
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