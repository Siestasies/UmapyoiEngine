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
    
    local playerTransform = GetTransform(playerId)
    local myTransform = GetTransform()
    if not playerTransform or not myTransform then
        return
    end
    
    local dx = playerTransform.position.x - myTransform.position.x
    local dy = playerTransform.position.y - myTransform.position.y
    local distSq = dx * dx + dy * dy
    
    -- Transition to Chase if player within range
    if distSq < ExposedVars.chaseEnterRange * ExposedVars.chaseEnterRange then
        ChangeState(entity, "WindDemonChase")
        return
    end
end

function state_exit(entity)
    Log("idle exit")
end
