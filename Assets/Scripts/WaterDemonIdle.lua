ExposedVars = {
    chaseEnterRange = 20.0
}

local animator

function state_enter(entity)
    Log("idle entered")

    if HasAnimator() then
        animator = GetAnimator()
    end
    animator.animator:Play("idle", true)
end

function state_update(entity, dt)
    local playerId = FindEntityWithComponent("Player")
    if playerId == -1 or not IsEntityValid(playerId) then
        return
    end
    
    local playerTransform = GetTransformFrom(playerId)
    local myTransform = GetTransform()
    if not playerTransform or not myTransform then
        return
    end
    
    local dx = playerTransform.worldPosition.x - myTransform.worldPosition.x
    local dy = playerTransform.worldPosition.y - myTransform.worldPosition.y
    local distSq = dx * dx + dy * dy

    --Log("Update Attack player world pos" .. playerTransform.worldPosition.x .. ", " .. playerTransform.worldPosition.y)
    Log("Update Idle " .. distSq .. " " .. "attackrange " .. ExposedVars.chaseEnterRange * ExposedVars.chaseEnterRange)
    
    -- Transition to Chase if player within range
    if distSq < ExposedVars.chaseEnterRange * ExposedVars.chaseEnterRange then
        ChangeState(entity, "WaterDemonChase")
        return
    end
end

function state_exit(entity)
    Log("idle exit")
end
