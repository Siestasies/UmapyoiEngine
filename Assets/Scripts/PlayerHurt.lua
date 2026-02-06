-- PlayerHurt.lua
-- Hurt state - triggered when player takes damage, provides i-frames

ExposedVars = {
    hurtAnimationName = "hurt",
    hurtDuration = 0.3,
    knockbackForce = 200.0
}

-- State-local variables
local hurtTimer = 0
local knockbackApplied = false
local audio = nil

function state_enter(entity)
    Log("Player entered Hurt state")
    
    if not HasPlayer() then
        ChangeState(entity, "PlayerIdle")
        return
    end
    
    local player = GetPlayer()
    if not player then
        ChangeState(entity, "PlayerIdle")
        return
    end
    
    -- Check if player is dead
    if player.mHealth <= 0 then
        ChangeState(entity, "PlayerDeath")
        return
    end

    if HasAudioComponent() then
        audio = GetAudioComponent()
    end
    
    -- Set hurt duration from player component or default
    hurtTimer = player.mHitStunDuration
    if hurtTimer <= 0 then
        hurtTimer = hurtDuration
    end
    
    -- Make player invulnerable during hurt animation
    player.isInvulnerable = true
    player.mInvulnerabilityDuration = hurtDuration + 0.5
    player.isStunned = true
    player.stunedTimer = hurtTimer
    
    -- Reset knockback flag
    knockbackApplied = false
    
    -- Play hurt animation and sound
    PlayAnimation(entity, hurtAnimationName)
    --PlaySound("player_hurt", 0.8, 0)
    audio:play(entity,"PlayerDamage")
    
    -- Stop current movement
    if HasRigidBody() then
        local rb = GetRigidBody()
        if rb then
            rb.velocity = Vec2(0, 0)
        end
    end
end

function state_update(entity, dt)
    if not HasPlayer() then
        ChangeState(entity, "PlayerIdle")
        return
    end
    
    local player = GetPlayer()
    if not player then
        ChangeState(entity, "PlayerIdle")
        return
    end
    
    -- Check if player died during hurt state
    if player.mHealth <= 0 then
        ChangeState(entity, "PlayerDeath")
        return
    end
    
    -- Apply knockback once at the start
    if not knockbackApplied then
        ApplyKnockback(entity)
        knockbackApplied = true
    end
    
    -- Update timer
    hurtTimer = hurtTimer - dt
    
    -- Reduce knockback velocity over time
    if HasRigidBody() then
        local rb = GetRigidBody()
        if rb then
            local decay = 5.0 * dt
            rb.velocity.x = rb.velocity.x * (1.0 - decay)
            rb.velocity.y = rb.velocity.y * (1.0 - decay)
        end
    end
    
    -- Hurt animation finished
    if hurtTimer <= 0 then
        -- Clear stun
        player.isStunned = false
        player.stunedTimer = 0
        
        -- Keep invulnerability for a bit longer
        -- (handled by player component's invulnerability timer)
        
        -- Transition back to idle
        ChangeState(entity, "PlayerIdle")
    end
end

function state_exit(entity)
    Log("Player exited Hurt state")
    
    if not HasPlayer() then return end
    
    local player = GetPlayer()
    if player then
        -- Start invulnerability timer (separate from hurt stun)
        -- Player will remain invulnerable for mInvulnerabilityDuration
        player.isStunned = false
    end
    
    -- Stop any remaining knockback
    if HasRigidBody() then
        local rb = GetRigidBody()
        if rb then
            rb.velocity = Vec2(0, 0)
        end
    end
    
    -- Reset state-local variables
    hurtTimer = 0
    knockbackApplied = false
end

-- Apply knockback away from the damage source
function ApplyKnockback(entity)
    if not HasRigidBody() then return end
    if not HasTransform() then return end
    
    local rb = GetRigidBody()
    local transform = GetTransform()
    
    if not rb or not transform then return end
    
    -- Try to find nearest enemy as knockback source
    local enemies = FindEntitiesWithComponent("Enemy")
    if not enemies or #enemies == 0 then
        -- No enemies found, just use a default knockback direction
        local sprite = GetSprite()
        local knockbackDir = 1
        if sprite and sprite.flipX then
            knockbackDir = 1  -- Facing left, knockback to right
        else
            knockbackDir = -1  -- Facing right, knockback to left
        end
        rb.velocity = Vec2(knockbackForce * knockbackDir, knockbackForce * 0.3)
        return
    end
    
    -- Find closest enemy
    local myPos = transform.position
    local closestDist = math.huge
    local closestPos = myPos
    
    for i = 1, #enemies do
        local enemyId = enemies[i]
        if IsEntityValid(enemyId) then
            local enemyTransform = GetTransformFrom(enemyId)
            if enemyTransform then
                local dx = enemyTransform.position.x - myPos.x
                local dy = enemyTransform.position.y - myPos.y
                local dist = math.sqrt(dx * dx + dy * dy)
                
                if dist < closestDist then
                    closestDist = dist
                    closestPos = enemyTransform.position
                end
            end
        end
    end
    
    -- Calculate knockback direction (away from enemy)
    local dx = myPos.x - closestPos.x
    local dy = myPos.y - closestPos.y
    local length = math.sqrt(dx * dx + dy * dy)
    
    if length > 0 then
        dx = dx / length
        dy = dy / length
    else
        dx = 1
        dy = 0
    end
    
    -- Apply knockback velocity
    rb.velocity = Vec2(dx * knockbackForce, dy * knockbackForce * 0.5)
end
