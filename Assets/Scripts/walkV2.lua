
ExposedVars = { speed = 50.0 }

function state_enter(entity)
    Log("WalkState entered")
    PlayEntitySound(entity, "footsteps", true)
end

function Update(dt)
    -- Pure walk behavior
    local transform = GetTransform(EntityID)
    transform.position.x = transform.position.x + ExposedVars.speed * dt
end

function state_exit(entity)
    StopEntitySoundByName(entity, "footsteps")
end
