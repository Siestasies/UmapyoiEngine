ExposedVars = {
    chaseExitRange = 45.0,    -- Increased from 25    -- Stop chasing when player gets far (LARGER than chaseEnter)
    attackEnterRange = 25.0   -- Increased from 8    -- Start attacking when close enough
}

local animator

function state_enter(entity)
    --add enter logic
    --entity is to reference code to this entity like
    --which transform to update 
    Log("Chase entered")

    if HasAnimator() then
        animator = GetAnimator()
        --animator.animator:Play("WaterDemonStun", true)
    end

    animator.animator:Play("walking", false)
end

function state_update(entity, dt)
    local playerId = FindEntityWithComponent("Player")

    if IsEntityValid(playerId) == false then
        ChangeState(entity, "WaterDemonIdle")
        return
    end
    
    local playerTransform = GetTransformFrom(playerId)
    local myTransform = GetTransformFrom(entity)  -- Changed this
    if not playerTransform or not myTransform then return end
    
    local dx = playerTransform.worldPosition.x - myTransform.worldPosition.x
    local dy = playerTransform.worldPosition.y - myTransform.worldPosition.y
    local distSq = dx * dx + dy * dy
    local dist = math.sqrt(distSq)  -- ADD THIS for easier debugging
    
    -- ADD DEBUG LOGGING
    --Log("Chase Update - Distance: " .. dist .. " | AttackRange: " .. ExposedVars.attackEnterRange .. " | ExitRange: " .. ExposedVars.chaseExitRange)
    
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
    
    -- Check attack transition
    if enemy and enemy.mAttackDamage ~= -1 and distSq <= ExposedVars.attackEnterRange * ExposedVars.attackEnterRange then
        Log("TRANSITIONING TO ATTACK - Distance: " .. dist)
        ChangeState(entity, "WaterDemonAttack")
        return
    end
    
    -- Check idle transition
    if distSq > ExposedVars.chaseExitRange * ExposedVars.chaseExitRange then
        Log("TRANSITIONING TO IDLE - Distance: " .. dist)
        ChangeState(entity, "WaterDemonIdle")
    end
end


function state_exit(entity)
    Log("Chase exit")

    if HasPathFinding() then 
        local pf = GetPathFinding()
        --pf.goal.x = GetTransform().worldPosition.x
        --pf.goal.y = GetTransform().worldPosition.y

        pf.reachedGoal = true
    end 

end