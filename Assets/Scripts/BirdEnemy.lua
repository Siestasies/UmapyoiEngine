ExposedVars = {
    speed = 100.0,
    name = "unknown",
    isActive = true
}

function Start()
    name = "bird"
end

function Update(dt)
    local transform = GetTransform()
    local rb = GetRigidBody()

    if transform and rb then 
        --rb.velocity.x = -200 * speed * dt
    else
        Log("components are missing");
    end

    if KeyPressed(KEY_W) then 
        Log("W IS PRESSED")
    elseif KeyDown(KEY_W) then 
        Log("W IS DOWN")
    elseif KeyReleased(KEY_W) then 
        Log("W IS RELEASE")
    end
    
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