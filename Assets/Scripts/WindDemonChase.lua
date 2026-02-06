ExposedVars = {
    chaseExitRange = 70.0,
    attackEnterRange = 40.0
}

local animator

function state_enter(entity)
    Log("Chase entered")

    if HasAnimator() then
        animator = GetAnimator()
    end

    animator.animator:Play("idle", false)
end

function state_update(entity, dt)
    --look for player entity
    local playerId = FindEntityWithComponent("Player")
    if not IsEntityValid(playerId) then
        ChangeState(entity, "WindDemonIdle")
        return
    end

    --get player and entity transform
    local playerTransform = GetTransformFrom(playerId)
    local myTransform = GetTransform()
    if not playerTransform or not myTransform then return end

    local dx = playerTransform.worldPosition.x - myTransform.worldPosition.x
    local dy = playerTransform.worldPosition.y - myTransform.worldPosition.y
    local distSq = dx * dx + dy * dy

    local enemy
    if HasEnemy() then
        enemy = GetEnemy()
    else
        Log("not enemy")
        return
    end

    if HasPathFinding() then 
        local pf = GetPathFinding()
        if pf then
            pf.goal.x = playerTransform.worldPosition.x
            pf.goal.y = playerTransform.worldPosition.y
        end
    end 

    --if enemy is within attack range
    if enemy and distSq <= ExposedVars.attackEnterRange * ExposedVars.attackEnterRange then
        ChangeState(entity, "WindDemonAttack")
        return
    end

    if distSq > ExposedVars.chaseExitRange * ExposedVars.chaseExitRange then
        ChangeState(entity, "WindDemonIdle")
    end
end

function state_exit(entity)
    Log("Chase exit")

    if HasPathFinding() then 
        local pf = GetPathFinding()

        pf.reachedGoal = true
    end 
end