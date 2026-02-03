ExposedVars = {
    chaseExitRange = 25.0,    -- Stop chasing when player gets far (LARGER than chaseEnter)
    attackEnterRange = 8.0    -- Start attacking when close enough
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
    --Log("player id : " .. playerId)
    --Log("Update Attack player world pos" .. playerTransform.worldPosition.x .. ", " .. playerTransform.worldPosition.y)
    
    local playerTransform = GetTransformFrom(playerId)
    local myTransform = GetTransform()
    if not playerTransform or not myTransform then return end
    
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
    
    if HasPathFinding() then 
        local pf = GetPathFinding()
        if pf then
            pf.goal.x = playerTransform.position.x
            pf.goal.y = playerTransform.position.y
        end
    end 
    
    if enemy and distSq <= ExposedVars.attackEnterRange * ExposedVars.attackEnterRange then
        ChangeState(entity, "WaterDemonAttack")
        return
    end
    
    if distSq > ExposedVars.chaseExitRange * ExposedVars.chaseExitRange then
        ChangeState(entity, "WaterDemonIdle")
    end
end


function state_exit(entity)
    Log("Chase exit")

    if HasPathFinding() then 
        local pf = GetPathFinding()
        pf.goal.x = GetTransform().worldPosition.x
        pf.goal.y = GetTransform().worldPosition.y

        pf.reachedGoal = true
    end 

end