ExposedVars = {
    ChaseRange = 8.0
}

function state_enter(entity)
    Log("idle entered")
end

function state_update(entity, dt)
    local playerId = FindEntityWithComponent("Player")
    if playerId == -1 or not IsEntityValid(playerId) then
        return
    end
    
    local playerTransform = GetTransform(playerId)
    local myTransform = GetTransform(entity)
    if not playerTransform or not myTransform then
        return
    end
    
    local dx = playerTransform.position.x - myTransform.position.x
    local dy = playerTransform.position.y - myTransform.position.y
    local distSq = dx * dx + dy * dy
    
    -- Transition to Chase if player within range
    if distSq < ExposedVars.ChaseRange * ExposedVars.ChaseRange then
        ChangeState(entity, "FireDemonChase")
        return
    end
end

function state_exit(entity)
    Log("idle exit")
end
