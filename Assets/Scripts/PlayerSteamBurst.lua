-- PlayerSteamBurst.lua
-- Steam Burst - powerful fusion attack requiring elemental combo (Fire + Water or Water + Fire)

ExposedVars = {
    steamBurstAnimationName = "steam_burst",
    attackDuration = 0.7,
    manaCost = 30,
    damageMultiplier = 2.5,
    aoeRadius = 100.0
}

-- State-local variables
local attackTimer = 0
local attackPerformed = false

function state_enter(entity)
    Log("Player entered Steam Burst state")
    
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
    
    -- Check elemental combo requirement
    if player.elementComboTimer <= 0 or player.lastElementUsed == ElementType.None then
        Log("Cannot use Steam Burst - no elemental combo active!")
        ChangeState(entity, "PlayerIdle")
        return
    end
    
    -- Check mana cost
    local actualManaCost = GetSteamBurstManaCost(player)
    if player.mMana < actualManaCost then
        Log("Not enough mana for Steam Burst!")
        ChangeState(entity, "PlayerIdle")
        return
    end
    
    -- Consume mana
    player.mMana = player.mMana - actualManaCost
    
    -- Clear elemental combo state (consumed by Steam Burst)
    player.lastElementUsed = ElementType.None
    player.elementComboTimer = 0
    
    -- Initialize attack
    attackTimer = attackDuration
    attackPerformed = false
    
    -- Play animation and sound
    PlayAnimation(entity, steamBurstAnimationName)
    PlaySound("steam_burst", 1.0, 0)
    
    -- Stop movement
    if HasRigidBody() then
        local rb = GetRigidBody()
        if rb then
            rb.velocity = Vec2(0, 0)
        end
    end
    
    Log("Steam Burst activated! AoE attack incoming!")
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
    
    -- Perform AoE damage at attack midpoint
    if not attackPerformed and attackTimer < (attackDuration * 0.4) then
        PerformSteamBurstDamage(entity, player)
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
    Log("Player exited Steam Burst state")
    
    -- Reset state-local variables
    attackTimer = 0
    attackPerformed = false
end

-- Get mana cost from attack stats or use default
function GetSteamBurstManaCost(player)
    if not player then return manaCost end
    
    local attackStats = player.attackStats
    if attackStats then
        for i = 1, #attackStats do
            local attack = attackStats[i]
            if attack and attack.elementType == ElementType.Steam then
                return attack.manaCost
            end
        end
    end
    
    return manaCost
end

-- Perform AoE steam burst damage
function PerformSteamBurstDamage(entity, player)
    if not player then return end
    
    local enemies = FindEntitiesWithComponent("Enemy")
    if not enemies then return end
    
    local myTransform = GetTransform()
    if not myTransform then return end
    
    local myPos = myTransform.position
    local baseDamage = player.mAttackDamage
    
    -- Get attack stats for steam burst
    local attackStats = player.attackStats
    local actualDamageMultiplier = damageMultiplier
    local actualAoeRadius = aoeRadius
    
    if attackStats then
        for i = 1, #attackStats do
            local attack = attackStats[i]
            if attack and attack.elementType == ElementType.Steam then
                actualDamageMultiplier = attack.mDamageMultiplier
                actualAoeRadius = attack.attackRange
                break
            end
        end
    end
    
    local totalDamage = math.floor(baseDamage * actualDamageMultiplier)
    local enemiesHit = 0
    
    -- Check ALL enemies in AoE radius
    for i = 1, #enemies do
        local enemyId = enemies[i]
        if IsEntityValid(enemyId) then
            local enemyTransform = GetTransformFrom(enemyId)
            if enemyTransform then
                local enemyPos = enemyTransform.position
                
                local dx = enemyPos.x - myPos.x
                local dy = enemyPos.y - myPos.y
                local distance = math.sqrt(dx * dx + dy * dy)
                
                -- AoE hits all enemies in radius
                if distance <= actualAoeRadius then
                    if HasEnemyOn(enemyId) then
                        local enemy = GetEnemyFrom(enemyId)
                        if enemy then
                            -- Apply steam damage (scales with distance - closer = more damage)
                            local distanceMultiplier = 1.0 - (distance / actualAoeRadius) * 0.5
                            local scaledDamage = math.floor(totalDamage * distanceMultiplier)
                            
                            enemy.mHealth = enemy.mHealth - scaledDamage
                            enemiesHit = enemiesHit + 1
                            
                            Log("Steam Burst hit enemy for " .. tostring(scaledDamage) .. " damage!")
                        end
                    end
                end
            end
        end
    end
    
    -- Play explosion sound
    PlaySound("steam_explosion", 0.9, 0)
    
    Log("Steam Burst hit " .. tostring(enemiesHit) .. " enemies!")
end
