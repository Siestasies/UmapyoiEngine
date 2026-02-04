-- PlayerFireSlash.lua
-- Fire Slash elemental attack - applies burn and sets up elemental combo

ExposedVars = {
    fireSlashAnimationName = "atk_3",
    fireSlashSoundName = "atk_3",
    attackDuration = 0.5,
    manaCost = 20,
    damageMultiplier = 1.5,
    burnDuration = 3.0
}

-- State-local variables
local attackTimer = 0
local comboAtkBuffer = 0
local attackPerformed = false

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
    
    -- Check mana cost
    local actualManaCost = GetFireSlashManaCost(player)
    if player.mMana < actualManaCost then
        Log("Not enough mana for Fire Slash!")
        ChangeState(entity, "PlayerIdle")
        return
    end
    
    -- Consume mana
    player.mMana = math.floor(player.mMana - actualManaCost)
    
    -- Set elemental combo state
    player.lastElementUsed = ElementType.Fire
    player.elementComboTimer = player.elementComboWindow
    
    -- Initialize attack
    attackTimer = attackDuration
    attackPerformed = false
    
    -- Play animation and sound
    PlayAnimation(entity, fireSlashAnimationName)
    PlaySound(fireSlashSoundName, 0.8, 0)
    
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

    -- Check for Water Slash (E key)
    if KeyPressed(KEY_E) and attackTimer > (attackDuration * 0.5) then
        if CanUseElementalAttack(player, "water") then
            StopSound(fireSlashSoundName)
            ChangeState(entity, "PlayerSteamBurst")
            return
        else
            Log("Not enough mana for Steam Burst!")
        end
    end
    
    -- Perform damage at attack midpoint
    if not attackPerformed and attackTimer < (attackDuration * 0.5) then
        PerformFireSlashDamage(entity, player)
        attackPerformed = true
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
    
    -- Reset state-local variables
    attackTimer = 0
    attackPerformed = false
end

-- Get mana cost from attack stats or use default
function GetFireSlashManaCost(player)
    if not player then return manaCost end
    
    local attackStats = player.attackStats
    if attackStats then
        for i = 1, #attackStats do
            local attack = attackStats[i]
            if attack and attack.elementType == ElementType.Fire then
                return attack.manaCost
            end
        end
    end
    
    return manaCost
end

-- Perform fire slash damage with burn effect
function PerformFireSlashDamage(entity, player)
    if not player then return end
    
    local enemies = FindEntitiesWithComponent("Enemy")
    if not enemies then return end
    
    local myTransform = GetTransform()
    if not myTransform then return end
    
    local myPos = myTransform.position
    local attackRange = player.mAttackRange * 1.2  -- Fire slash has slightly more range
    local baseDamage = player.mAttackDamage
    
    -- Get attack stats for fire slash
    local attackStats = player.attackStats
    local actualDamageMultiplier = damageMultiplier
    local applyBurn = true
    
    if attackStats then
        for i = 1, #attackStats do
            local attack = attackStats[i]
            if attack and attack.elementType == ElementType.Fire then
                actualDamageMultiplier = attack.mDamageMultiplier
                attackRange = attack.attackRange
                applyBurn = attack.applyBurn
                break
            end
        end
    end
    
    local totalDamage = math.floor(baseDamage * actualDamageMultiplier)
    
    -- Check each enemy
    for i = 1, #enemies do
        local enemyId = enemies[i]
        if IsEntityValid(enemyId) then
            local enemyTransform = GetTransformFrom(enemyId)
            if enemyTransform then
                local enemyPos = enemyTransform.position
                
                local dx = enemyPos.x - myPos.x
                local dy = enemyPos.y - myPos.y
                local distance = math.sqrt(dx * dx + dy * dy)
                
                if distance <= attackRange then
                    if HasEnemyOn(enemyId) then
                        local enemy = GetEnemyFrom(enemyId)
                        if enemy then
                            -- Apply fire damage
                            enemy.mHealth = enemy.mHealth - totalDamage
                            Log("Fire Slash hit enemy for " .. tostring(totalDamage) .. " damage!")
                            
                            -- Play hit sound
                            PlaySound("fire_hit", 0.8, 0)
                            
                            -- TODO: Apply burn effect (would need enemy status effect system)
                            if applyBurn then
                                Log("Enemy is burning for " .. tostring(burnDuration) .. " seconds!")
                            end
                        end
                    end
                end
            end
        end
    end
end

-- Helper function to face towards mouse
function FaceTowardsMouse(entity)
    if not HasTransform() then return end
    if not HasSprite() then return end
    
    local transform = GetTransform()
    local sprite = GetSprite()
    
    if not transform or not sprite then return end
    
    local mousePos = GetMousePosition()
    local myPos = transform.position
    
    if mousePos.x < myPos.x then
        sprite.flipX = true
    else
        sprite.flipX = false
    end
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
            end
        end
    end
    
    -- Default mana check if no specific attack found
    local defaultManaCost = 20
    return player.mMana >= defaultManaCost
end