-- PlayerRun.lua
-- Run/Movement state - handles WASD movement

ExposedVars = {
    runAnimationName = "walk"
}

local animator = nil

function state_enter(entity)
    Log("Player entered Run state")
    
    -- Play run animation
    if HasAnimator() then
        animator = GetAnimator()
        Log("IM RUNNNNNNING")
    end

    animator.animator:Play("walk", false)
    
end

function state_update(entity, dt)
    if not HasPlayer() then return end
    if not HasRigidBody() then return end
    if not HasTransform() then return end
    
    local player = GetPlayer()
    local rb = GetRigidBody()
    local transform = GetTransform()
    
    if not player or not rb or not transform then return end
    
    -- Check if stunned
    if player.isStunned then
        rb.velocity = Vec2(0, 0)
        ChangeState(entity, "PlayerIdle")
        return
    end
    
    local targetAccel = Vec2(0, 0)

    local targetVel = Vec2(0, 0)
    local speed = 50

    if KeyDown(KEY_W) then targetVel.y = targetVel.y + speed end
    if KeyDown(KEY_S) then targetVel.y = targetVel.y - speed end
    if KeyDown(KEY_A) then targetVel.x = targetVel.x - speed end
    if KeyDown(KEY_D) then targetVel.x = targetVel.x + speed end

    if targetVel.x == 0 and targetVel.y == 0 then
        rb.velocity = Vec2(0, 0)
        ChangeState(entity, "PlayerIdle")
        return
    end

    -- Normalize for diagonal movement
    if targetVel.x ~= 0 and targetVel.y ~= 0 then
        targetVel = targetVel * 0.7071 -- 1/sqrt(2)
    end

rb.velocity = targetVel
    
    -- Flip sprite based on direction
    if rb.velocity.x < 0 and transform.scale.x > 0 then
       transform.scale.x = -1.0 * transform.scale.x
    elseif rb.velocity.x > 0 and transform.scale.x < 0 then
        transform.scale.x = -1.0 * transform.scale.x
    end
    
    -- Check for attack input while moving
    if MouseButtonPressed(MOUSE_LEFT) then
        ChangeState(entity, "PlayerAttack")
        return
    end
    
    if KeyPressed(KEY_Q) and KeyPressed(KEY_E) then
        ChangeState(entity, "PlayerSteamBurst")
        return
    end
    
    -- Check for elemental attacks
    if KeyPressed(KEY_Q) then
        ChangeState(entity, "PlayerFireSlash")
        return
    end
    
    if KeyPressed(KEY_E) then
        ChangeState(entity, "PlayerWaterSlash")
        return
    end

    if KeyPressed(KEY_R) then
        ChangeState(entity, "PlayerWindDash")
        return
    end
    
end

function state_exit(entity)
    Log("Player exited Run state")
end
