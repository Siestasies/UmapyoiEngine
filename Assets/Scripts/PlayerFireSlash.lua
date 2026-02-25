-- PlayerFireSlash.lua
-- Fire Slash elemental attack - applies burn and sets up elemental combo
local audio = nil

ExposedVars = {
    fireSlashAnimationName = "atk_3",
    fireSlashSoundName = "atk_3",
    attackDuration = 0.5
}

-- State-local variables
local attackTimer = 0
local attackPerformed = false
local attackStat = nil
local animator = nil
local collider = nil

function state_enter(entity)
    Log("Player entered Fire Slash state")
    
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
    player.currAttackIndex = 2
    
    -- Check mana cost
    attackStat = GetFireSlashAttackStat(player)
    if player.mMana < attackStat.manaCost then
        Log("Not enough mana for Fire Slash!")
        ChangeState(entity, "PlayerIdle")
        return
    end
    
    -- Consume mana
    player.mMana = math.floor(player.mMana - attackStat.manaCost)

    local transform = GetTransformFrom(EntityID)
    SpawnFeedback(transform.worldPosition.x, transform.worldPosition.y + 10, tostring(attackStat.manaCost), "manaspend")
    
    -- Set elemental combo state
    --player.lastElementUsed = ElementType.Fire
    --player.elementComboTimer = player.elementComboWindow
    
    -- Initialize attack
    attackTimer = attackDuration
    attackPerformed = false
    
    -- Play animation and sound
    animator.animator:Play(fireSlashAnimationName, true)
    --PlaySound(fireSlashSoundName, 0.8, 0)
    audio = GetAudioComponent()
    audio:play(EntityID, "FireSlash")
    Log("Player has died!")
    
    -- Stop movement
    if HasRigidBody() then
        local rb = GetRigidBody()
        if rb then
            rb.velocity = Vec2(0, 0)
        end
    end
    
    -- Face towards mouse
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
    
    -- Update timer
    attackTimer = attackTimer - dt
    local transform = GetTransformFrom(EntityID)

    if KeyPressed(KEY_R) and attackTimer > (attackDuration * 0.5) then
        -- Check if player has enough mana for wind dash
        if CanUseElementalAttack(player, "wind") then
            ChangeState(entity, "PlayerPyronado")
            return
        else
            Log("Not enough mana for Pyronado!")
            SpawnFeedback(transform.worldPosition.x, transform.worldPosition.y + 10, "Not enough mana for Pyronado!", "warning")            
        end
    end

    -- Check for Water Slash (E key)
    if KeyPressed(KEY_E) and attackTimer > (attackDuration * 0.5) then
        if CanUseElementalAttack(player, "water") then
            ChangeState(entity, "PlayerSteamBurst")
            return
        else
            Log("Not enough mana for Steam Burst!")
            SpawnFeedback(transform.worldPosition.x, transform.worldPosition.y + 10, "Not enough mana for Steam Burst!", "warning")            
        end
    end
    
    -- Perform attack at animation midpoint
    if not attackPerformed and attackTimer < (attackDuration * 0.4) then
        Log("Fire Slash Attack!")
        attackPerformed = true
        -- Activate Corresponding Collider
        collider.shapes[attackStat.triggerColliderIndex+2].isActive = true
    end
    
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
    Log("Player exited Fire Slash state")
    if attackStat then 
        collider.shapes[attackStat.triggerColliderIndex+2].isActive = false
    end
    StopSound(fireSlashSoundName);
end

function GetFireSlashAttackStat(player)
    if not player then return nil end
    
    local attackStats = player.attackStats
    if attackStats then
        for i = 1, #attackStats do
            local attack = attackStats[i]
            if attack and attack.elementType == ElementType.Fire then
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