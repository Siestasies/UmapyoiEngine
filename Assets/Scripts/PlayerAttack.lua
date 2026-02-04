-- PlayerAttack.lua
-- Basic attack state - handles neutral attack combos (Attack 1, Attack 2)

ExposedVars = {
    attack1AnimationName = "normal_atk",
    attack2AnimationName = "normal_atk",
    attackDuration = 0.4,
    comboWindowDuration = 0.3
}

-- State-local variables
local attackTimer = 0
local comboTimer = 0
local currentCombo = 1  -- 1 = first attack, 2 = second attack
local attackPerformed = false
local canCombo = false
local animator = nil
local collider = nil

function state_enter(entity)
    Log("Player entered Attack state")
    
    if not HasPlayer() then
        ChangeState(entity, "PlayerIdle")
        return
    end
    
    local player = GetPlayer()
    if not player then
        ChangeState(entity, "PlayerIdle")
        return
    end
    
    -- Check if stunned
    if player.isStunned then
        ChangeState(entity, "PlayerIdle")
        return
    end
    
    -- Initialize attack
    attackTimer = attackDuration
    comboTimer = 0
    attackPerformed = false
    canCombo = false
    
    -- Determine which attack in combo (based on player's current attack index)
    currentCombo = player.currAttackIndex + 1
    if currentCombo > 2 then
        currentCombo = 1
    end

    if HasAnimator() then
        animator = GetAnimator()
    end
    
    if HasCollider() then
        collider = GetCollider()
    end

    -- Play appropriate animation
    if currentCombo == 1 then
        Log("atk1")
        animator.animator:Play(attack1AnimationName, true)
        --PlaySound("attack_1", 0.8, 0)
    else
        animator.animator:Play(attack2AnimationName, true)
        --PlaySound("attack_2", 0.8, 0)
    end

    collider.shapes[3].isActive = true

    -- Stop movement during attack
    if HasRigidBody() then
        local rb = GetRigidBody()
        if rb then
            rb.velocity = Vec2(0, 0)
        end
    end
    
    -- Face towards mouse position
    FaceTowardsMouse(entity)
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
    
    -- Update timers
    attackTimer = attackTimer - dt
    
    -- Perform damage check at attack midpoint (if not already done)
    if not attackPerformed and attackTimer < (attackDuration * 0.5) then
        PerformAttackDamage(entity, player)
        attackPerformed = true
        canCombo = true  -- Enable combo window after hit
    end
    
    -- Check for combo input during combo window
    if canCombo and MouseButtonPressed(MOUSE_LEFT) then
        -- Queue up next attack in combo
        player.currAttackIndex = currentCombo  -- Will be incremented on next state_enter
        comboTimer = comboWindowDuration
    end
    
    -- Attack animation finished
    if attackTimer <= 0 then
        -- Check if combo was queued
        if comboTimer > 0 then
            -- Immediately go to next attack
            ChangeState(entity, "PlayerAttack")
            return
        end
        
        -- Reset combo
        player.currAttackIndex = 0
        if collider == nil and HasCollider() then
            collider = GetCollider()
        end
        collider.shapes[3].isActive = false
        
        -- Check for movement input to transition smoothly
        local moveX = 0
        local moveY = 0
        
        if KeyDown(KEY_W) then moveY = moveY + 1 end
        if KeyDown(KEY_S) then moveY = moveY - 1 end
        if KeyDown(KEY_A) then moveX = moveX - 1 end
        if KeyDown(KEY_D) then moveX = moveX + 1 end
        
        if moveX ~= 0 or moveY ~= 0 then
            ChangeState(entity, "PlayerRun")
        else
            ChangeState(entity, "PlayerIdle")
        end
    end
end

function state_exit(entity)
    Log("Player exited Attack state")

    collider.shapes[3].isActive = false
    
    -- Reset state-local variables
    attackTimer = 0
    comboTimer = 0
    attackPerformed = false
    canCombo = false
end

-- Helper function to perform attack damage
function PerformAttackDamage(entity, player)
    if not player then return end
    
    -- Find enemies in attack range
    local enemies = FindEntitiesWithComponent("Enemy")
    if not enemies then return end
    
    local myTransform = GetTransform()
    if not myTransform then return end
    
    local myPos = myTransform.position
    local attackRange = player.mAttackRange
    local attackDamage = player.mAttackDamage
    
    -- Get damage multiplier from attack stats if available
    local attackStats = player.attackStats
    if attackStats and #attackStats >= currentCombo then
        local attack = attackStats[currentCombo]
        if attack then
            attackDamage = math.floor(player.mAttackDamage * attack.mDamageMultiplier)
            attackRange = attack.attackRange
        end
    end


    
    -- Check each enemy
    --for i = 1, #enemies do
    --    local enemyId = enemies[i]
    --    if IsEntityValid(enemyId) then
    --        local enemyTransform = GetTransformFrom(enemyId)
    --        if enemyTransform then
    --            local enemyPos = enemyTransform.position
    --            
    --            -- Calculate distance
    --            local dx = enemyPos.x - myPos.x
    --            local dy = enemyPos.y - myPos.y
    --            local distance = math.sqrt(dx * dx + dy * dy)
    --            
    --            -- Check if in range
    --            if distance <= attackRange then
    --                -- Deal damage to enemy
    --                if HasEnemyOn(enemyId) then
    --                    local enemy = GetEnemyFrom(enemyId)
    --                    if enemy then
    --                        enemy.mHealth = enemy.mHealth - attackDamage
    --                        Log("Hit enemy for " .. tostring(attackDamage) .. " damage!")
    --                        
    --                        -- Play hit sound
    --                        PlaySound("hit", 0.7, 0)
    --                        
    --                        -- Chance to gain mana on neutral attack hit
    --                        if math.random(1, 2) == 1 then
    --                            player.mMana = math.min(player.mMana + player.mNeutralAttackManaGain, player.mMaxMana)
    --                            Log("Gained " .. tostring(player.mNeutralAttackManaGain) .. " mana!")
    --                        end
    --                    end
    --                end
    --            end
    --        end
    --    end
    --end
end

-- Helper function to face towards mouse
function FaceTowardsMouse(entity)
    if not HasTransform() then return end
    if not HasSprite() then return end
    
    local transform = GetTransform()
    --local sprite = GetSprite()
    
    if not transform then return end
    
    local mousePos = GetMouseWorldPosition()
    local myPos = transform.position
    
    -- Determine facing direction based on mouse position
    if mousePos.x < myPos.x and transform.scale.x > 0 then
        --sprite.flipX = true
       transform.scale.x = -1.0 * transform.scale.x
    elseif mousePos.x > myPos.x and transform.scale.x < 0 then
        --sprite.flipX = false
        --myScale.x = 1.0 * myScale.x
        transform.scale.x = -1.0 * transform.scale.x
    end
end
