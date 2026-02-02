-- PlayerWaterSlash.lua
-- Water Slash elemental attack - can stun certain enemies, sets up elemental combo

ExposedVars = {
    waterSlashAnimationName = "water_slash",
    waterSlashSoundName = "water_slash",
    attackDuration = 0.5,
    manaCost = 20,
    damageMultiplier = 1.3,
    stunDuration = 1.5
}

-- State-local variables
local attackTimer = 0
local attackPerformed = false

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
    
    -- Check mana cost
    local actualManaCost = GetWaterSlashManaCost(player)
    if player.mMana < actualManaCost then
        Log("Not enough mana for Water Slash!")
        ChangeState(entity, "PlayerIdle")
        return
    end
    
    -- Consume mana
    player.mMana = math.floor(player.mMana - actualManaCost)
    
    -- Set elemental combo state
    player.lastElementUsed = ElementType.Water
    player.elementComboTimer = player.elementComboWindow
    
    -- Initialize attack
    attackTimer = attackDuration
    attackPerformed = false
    
    -- Play animation and sound
    PlayAnimation(entity, waterSlashAnimationName)
    PlaySound(waterSlashSoundName, 0.8, 0)
    
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

    -- Check for Fire Slash (Q key)
    if KeyPressed(KEY_Q) and attackTimer > (attackDuration * 0.5) then
        if CanUseElementalAttack(player, "fire") then
            StopSound(waterSlashSoundName)
            ChangeState(entity, "PlayerSteamBurst")
            return
        else
            Log("Not enough mana for Steam Burst!")
        end
    end
    
    -- Perform damage at attack midpoint
    if not attackPerformed and attackTimer < (attackDuration * 0.5) then
        PerformWaterSlashDamage(entity, player)
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
    Log("Player exited Water Slash state")
    
    -- Reset state-local variables
    attackTimer = 0
    attackPerformed = false
end

-- Get mana cost from attack stats or use default
function GetWaterSlashManaCost(player)
    if not player then return manaCost end
    
    local attackStats = player.attackStats
    if attackStats then
        for i = 1, #attackStats do
            local attack = attackStats[i]
            if attack and attack.elementType == ElementType.Water then
                return attack.manaCost
            end
        end
    end
    
    return manaCost
end

-- Perform water slash damage with potential stun
function PerformWaterSlashDamage(entity, player)
    if not player then return end
    
    local enemies = FindEntitiesWithComponent("Enemy")
    if not enemies then return end
    
    local myTransform = GetTransform()
    if not myTransform then return end
    
    local myPos = myTransform.position
    local attackRange = player.mAttackRange * 1.1
    local baseDamage = player.mAttackDamage
    
    -- Get attack stats for water slash
    local attackStats = player.attackStats
    local actualDamageMultiplier = damageMultiplier
    local applyStun = true
    local actualStunDuration = stunDuration
    
    if attackStats then
        for i = 1, #attackStats do
            local attack = attackStats[i]
            if attack and attack.elementType == ElementType.Water then
                actualDamageMultiplier = attack.mDamageMultiplier
                attackRange = attack.attackRange
                applyStun = attack.applyStun
                actualStunDuration = attack.effectDuration
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
                            -- Apply water damage
                            enemy.mHealth = enemy.mHealth - totalDamage
                            Log("Water Slash hit enemy for " .. tostring(totalDamage) .. " damage!")
                            
                            -- Play hit sound
                            PlaySound("water_hit", 0.8, 0)
                            
                            -- Apply stun effect
                            -- TODO: This would need enemy FSM integration
                            if applyStun then
                                Log("Enemy stunned for " .. tostring(actualStunDuration) .. " seconds!")
                                -- Trigger stun state on enemy FSM
                                -- ChangeState(enemyId, "EnemyStunned")
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