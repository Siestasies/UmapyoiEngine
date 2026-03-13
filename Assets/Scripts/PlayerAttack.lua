-- PlayerAttack.lua
-- Basic attack state - handles neutral attack combos (Attack 1, Attack 2)
local audio = nil

ExposedVars = {
    attack1AnimationName = "normal_atk",
    attack2AnimationName = "normal_atk",
    attackSoundnName = "normal_atk",
    attackDuration = 0.4,
    comboWindowDuration = 0.3
}

-- State-local variables
local attackTimer = 0
local comboTimer = 0
local currentCombo = 1  -- 1 = first attack, 2 = second attack
local attackPerformed = false
local canCombo = false
local attackStat = nil
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

    attackStat = GetAttackStat(player)

    -- Play appropriate animation
    if currentCombo == 1 then
        Log("atk1")
        animator.animator:Play(attack1AnimationName, true)
        --PlaySound("attack_1", 0.8, 0)
    else
        animator.animator:Play(attack2AnimationName, true)
        --PlaySound("attack_2", 0.8, 0)
    end
    PlaySound(attackSoundnName, 0.8, 0)

    -- Stop movement during attack
    if HasRigidBody() then
        local rb = GetRigidBody()
        if rb then
            rb.velocity = Vec2(0, 0)
        end
    end
    
    -- Face towards mouse position
    FaceTowardsMouse(entity)

    local attackDir = getAttackDirection(player)

    -- Activate Corresponding Collider
    collider.shapes[attackStat.triggerColliderIndex+2].isActive = true
    
    -- Apply attack/dash velocity
    local moveSpeed = player.mSpeed * 0.2
    if HasRigidBody() then
        local rb = GetRigidBody()
        if rb then
            rb.velocity = Vec2(attackDir.x * moveSpeed, attackDir.y * moveSpeed)
        end
    end

    audio = GetAudioComponent()
    audio:play(EntityID, "Neutral Slash(BasicAttack)")
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

    if not attackStat then 
        return
    end
    
    -- Update timers
    attackTimer = attackTimer - dt
    
    ---- Check for combo input during combo window
    --if canCombo and MouseButtonPressed(MOUSE_LEFT) then
    --    -- Queue up next attack in combo
    --    player.currAttackIndex = currentCombo  -- Will be incremented on next state_enter
    --    comboTimer = comboWindowDuration
    --end
    
    -- Attack animation finished
    if attackTimer <= 0 then
        ---- Check if combo was queued
        --if comboTimer > 0 then
        --    -- Immediately go to next attack
        --    ChangeState(entity, "PlayerAttack")
        --    return
        --end
        --
        ---- Reset combo
        --player.currAttackIndex = 0
        --if collider == nil and HasCollider() then
        --    collider = GetCollider()
        --end
        --collider.shapes[attackStat.triggerColliderIndex+2].isActive = false
        
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
    if attackStat then 
        collider.shapes[attackStat.triggerColliderIndex+2].isActive = false
    end
    StopSound(attackSoundnName)
end

function GetAttackStat(player)
    if not player then return nil end
    
    local attackStats = player.attackStats
    if attackStats then
        for i = 1, #attackStats do
            local attack = attackStats[i]
            if attack and attack.elementType == ElementType.None then
                return attack
            end
        end
    end
    
    return nil
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

-- Helper function to move player slightly towards mouse when attacking
function getAttackDirection(player)
    if not HasTransform() then return end
    if not HasSprite() then return end
    
    local transform = GetTransform()
    --local sprite = GetSprite()
    
    if not transform then return end
    
    local mousePos = GetMouseWorldPosition()
    local myPos = transform.position
    local direction = Vec2(1, 0)
    
    -- Determine direction based on mouse position and player postion
    direction = mousePos - myPos
    
    -- Normalize direction
    local length = math.sqrt(direction.x * direction.x + direction.y * direction.y)
    if length > 0 then
        direction = Vec2(direction.x / length, direction.y / length)
    end

    return direction
end