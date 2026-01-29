
ExposedVars = { speed = 50.0 }

function state_enter(entity)
    Log("WalkState entered")
end

function state_update(entity, dt)
    -- Pure walk behavior
    local transform = GetTransform(entity)
    transform.position.x = transform.position.x + ExposedVars.speed * dt
end

function state_exit(entity)
    Log("WalkState exited")
end
