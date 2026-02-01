ExposedVars = {
    ChaseRange = 30.0
}

function state_enter(entity)
    --add enter logic
    --entity is to reference code to this entity like
    --which transform to update 
    Log("Chase entered")
end

function state_update(entity, dt)
    --look for player entity
    local playerId = FindEntityWithComponent("Player")
    if playerId == -1 or not IsEntityValid(playerId) then
        return
    end

    --get player and entity transform
    local playerTransform = GetTransformFrom(playerId)
    local myTransform = GetTransform()
    if not playerTransform or not myTransform then
        return
    end

    local dx = playerTransform.position.x - myTransform.position.x
    local dy = playerTransform.position.y - myTransform.position.y
    local distSq = dx * dx + dy * dy

    local enemy
    if HasEnemy() then
        enemy = GetEnemy()
    else
        Log("not enemy")
        return
    end

    if distSq > enemy.mAttackRange * enemy.mAttackRange and 
       distSq <= ExposedVars.ChaseRange * ExposedVars.ChaseRange then

        if HasPathFinding() then 
            local pf = GetPathFinding()
            if pf then
                pf.goal.x = playerTransform.position.x
                pf.goal.y = playerTransform.position.y
            end
        end 
    end

    --if enemy is within attack range
    if HasEnemy() then
        local enemy = GetEnemy()
        if enemy then
            if distSq <= enemy.mAttackRange * enemy.mAttackRange then
                ChangeState(entity, "AttackState")
            end
        end
    end
end

function state_exit(entity)
    Log("Chase exit")
end