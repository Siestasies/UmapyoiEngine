
ExposedVars = { speed = 50.0 }

function state_enter(entity)
    Log("IdleState entered")
end

function state_update(entity, dt)
    -- Pure walk behavior
    -- local transform = GetTransform(EntityID)
    -- transform.position.x = transform.position.x + ExposedVars.speed * dt
end

function state_exit(entity)
    Log("Idle exit")
end
