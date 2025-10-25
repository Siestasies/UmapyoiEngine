ExposedVars = {
    speed = 100.0,
    health = 50,
    isActive = true
}

function Start()
    Log("kappa")
end

function Update(dt)
    local transform = GetTransform()
    local rb = GetRigidBody()
    
    rb.velocity.x = -200 * speed * dt
    --transform.scale = 1.1 * transform.scale
    
    --Log("Enemy position: " .. transform.position.x)
end

function OnCollision()
    log("Collided")
end