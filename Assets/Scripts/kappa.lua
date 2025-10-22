ExposedVars = {
    speed = 100.0,
    health = 50,
    isActive = true
}

function Start()
    Log("Enemy started!")
end

function Update(dt)
    local transform = GetTransform()
    local rb = GetRigidBody()
    
    rb.velocity.x = speed * dt
    
    Log("Enemy position: " .. transform.position.x)
end