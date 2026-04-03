-- PlayerWaterSlash.lua
-- Water Slash elemental attack - can stun certain enemies, sets up elemental combo
local audio = nil
local PlayerStatTrackState = require("PlayerStatTrackState")

ExposedVars = {
    waterSlashAnimationName = "water_slash",
    waterSlashSoundName = "water_slash",
    attackDuration = 0.35,
    ComboActivationFrame = 3
}

-- State-local variables
local attackTimer = 0
local attackPerformed = false
local attackStat = nil
local animator = nil
local collider = nil

function state_enter(entity)
    Log("Player entered Water Slash state")
    
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

    if HasAnimator() then
        animator = GetAnimator()
    end
    
    if HasCollider() then
        collider = GetCollider()
    end

    -- AttackIndex
    player.currAttackIndex = 3
    
    -- Check mana cost
    attackStat = GetWaterSlashAttackStat(player)
    if player.mMana < attackStat.manaCost then
        Log("Not enough mana for Water Slash!")
        ChangeState(entity, "PlayerIdle")
        return
    end
    
    -- Consume mana
    player.mMana = math.floor(player.mMana - attackStat.manaCost)
    local transform = GetTransformFrom(EntityID)
    SpawnFeedback(transform.worldPosition.x, transform.worldPosition.y + 10, tostring(attackStat.manaCost), "manaspend")
    
    -- Set elemental combo state
    --player.lastElementUsed = ElementType.Water
    --player.elementComboTimer = player.elementComboWindow
    
    -- Initialize attack
    attackTimer = attackDuration
    attackPerformed = false
    
    -- Play animation and sound
    animator.animator:Play(waterSlashAnimationName, true)
    audio = GetAudioComponent()
    
    local rand = math.random(2)
    if rand == 1 then
        audio:play(EntityID, "WaterSlash")
    elseif rand == 2 then
        audio:play(EntityID, "WaterSlash2")
    end
    -- Stop movement
    if HasRigidBody() then
        local rb = GetRigidBody()
        if rb then
            rb.velocity = Vec2(0, 0)
        end
    end
    
    -- Face towards mouse
    --FaceTowardsMouse(entity)

    attackDir = getAttackDirection(player)

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

    PlayerStatTrackState.incrWaterAttack()
    Log("Water Slash Attack!")
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

    local transform = GetTransformFrom(EntityID)
    
    -- Update timer
    attackTimer = attackTimer - dt

    if (KeyPressed(KEY_L) or GetControllerButtonInput(BTN_A, BTN_PRESS, 0)) 
    and animator.animator:GetCurrentFrame() >= ComboActivationFrame then
        -- Check if player has enough mana for wind dash
        if CanUseElementalAttack(player, "wind") then
            ChangeState(entity, "PlayerWhirlpool")
            return
        else
            Log("Not enough mana for Whirlpool!")
            SpawnFeedback(transform.worldPosition.x, transform.worldPosition.y + 10, "Not enough mana for Whirlpool!", "warning")                        
        end
    end

    -- Check for Fire Slash (Q key)
    if (KeyPressed(KEY_K) or GetControllerButtonInput(BTN_B, BTN_PRESS, 0))
    and animator.animator:GetCurrentFrame() >= ComboActivationFrame then
        if CanUseElementalAttack(player, "fire") then
            ChangeState(entity, "PlayerSteamBurst")
            return
        else
            Log("Not enough mana for Steam Burst!")
            SpawnFeedback(transform.worldPosition.x, transform.worldPosition.y + 10, "Not enough mana for Steam Burst!", "warning")
        end
    end
    
    --[[ Perform attack at animation midpoint
    if not attackPerformed and ColliderActivationFrame >= animator.animator:GetCurrentFrame() then
        Log("Water Slash Attack!")
        attackPerformed = true
        -- Activate Corresponding Collider
        collider.shapes[attackStat.triggerColliderIndex+2].isActive = true
    end]]
    
    -- Attack finished
    if attackTimer <= 0 then
        -- Check for movement input
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
    Log("Player exited Water Slash state")
    if attackStat then 
        collider.shapes[attackStat.triggerColliderIndex+2].isActive = false
    end
    StopSound(waterSlashSoundName);
end

function GetWaterSlashAttackStat(player)
    if not player then return nil end
    
    local attackStats = player.attackStats
    if attackStats then
        for i = 1, #attackStats do
            local attack = attackStats[i]
            if attack and attack.elementType == ElementType.Water then
                return attack
            end
        end
    end
    
    return nil
end

-- Helper function to check if elemental attack can be used
function CanUseElementalAttack(player, elementType)
    if not player then return false end
    
    -- Find the attack stats for this element
    local attackStats = player.attackStats
    if not attackStats then return false end
    
    for i = 1, #attackStats do
        local attack = attackStats[i]
        if attack then
            -- Check element type and mana cost
            if elementType == "fire" and attack.elementType == ElementType.Fire then
                return player.mMana >= attack.manaCost
            elseif elementType == "water" and attack.elementType == ElementType.Water then
                return player.mMana >= attack.manaCost
            elseif elementType == "wind" and attack.elementType == ElementType.Wind then
                return player.mMana >= attack.manaCost
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
    --local shape = collider.shapes[1]
    
    if not HasTransform() then return end
    if not HasSprite() then return end
    
    local transform = GetTransform()
    --local sprite = GetSprite()
    
    if not transform then return end
    
    --local mousePos = GetMouseWorldPosition()
    local myPos = transform.position
    --local myPos = Vec2(transform.position.x, transform.position.y + shape.offset.y)
    local direction = Vec2(transform.scale.x, 0)
    
    -- Determine direction based on mouse position and player postion
    --direction = mousePos - myPos
    
    -- Normalize direction
    local length = math.sqrt(direction.x * direction.x + direction.y * direction.y)
    if length > 0 then
        direction = Vec2(direction.x / length, direction.y / length)
    end

    return direction
end