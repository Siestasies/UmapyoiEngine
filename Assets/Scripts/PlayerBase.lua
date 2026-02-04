-- PlayerBase.lua
-- Base script for player entity - handles health, mana regen, and death transitions

ExposedVars = {
    debugMode = false,
    
    -- Basic Attack 1 settings
    attack1_damage = 1.0,
    attack1_speed = 1.0,
    attack1_range = 50.0,
    attack1_arc = 90.0,
    
    -- Basic Attack 2 settings
    attack2_damage = 1.2,
    attack2_speed = 0.9,
    attack2_range = 55.0,
    attack2_arc = 90.0,
    
    -- Fire Slash settings
    fireSlash_damage = 1.5,
    fireSlash_speed = 0.8,
    fireSlash_range = 60.0,
    fireSlash_manaCost = 20,
    fireSlash_burnDuration = 3.0,
    
    -- Water Slash settings
    waterSlash_damage = 1.3,
    waterSlash_speed = 0.85,
    waterSlash_range = 55.0,
    waterSlash_manaCost = 20,
    waterSlash_stunDuration = 1.5,
    
    -- Steam Burst settings
    steamBurst_damage = 2.5,
    steamBurst_speed = 0.6,
    steamBurst_range = 100.0,
    steamBurst_manaCost = 30
}

function Start()
    if HasPlayer() then
        local player = GetPlayer()
        if player then
            Log("PlayerBase initialized - Health: " .. tostring(player.mHealth) .. "/" .. tostring(player.mMaxHealth))
            ChangeState(EntityID, "PlayerIdle")
            InitializeAttackStats(player)
        end
    else
        LogWarning("PlayerBase: No Player component found!")
    end
end

function Update(dt)
    if not HasPlayer() then return end
    
    local player = GetPlayer()
    if not player then return end
    
    -- Check for death condition
    if player.mHealth <= 0 then
        ChangeState(EntityID, "PlayerDeath")
        return
    end
    
    updateTimers(dt)

    -- Check for stun condition
    if player.isStunned then
        player.stunedTimer = player.stunedTimer - dt
        if player.stunedTimer <= 0 then
            player.isStunned = false
            player.stunedTimer = 0
        end
    end
    
    -- Handle mana regeneration (always active)
    if player.mMana < player.mMaxMana then
        local manaGain = player.mManaRegenRate * dt
        player.mMana = math.min(player.mMana + manaGain, player.mMaxMana)
    end
    
    -- Debug logging
    if debugMode then
        Log("Player Health: " .. tostring(player.mHealth) .. " Mana: " .. tostring(math.floor(player.mMana)))
    end
end

function OnDestroy()
    Log("PlayerBase destroyed")
end

-- Collision handling for damage
function OnCollisionEnter(other)
    if not HasPlayer() then return end
    
    local player = GetPlayer()
    if not player then return end
    
    -- Check invulnerability
    if player.isInvulnerable then return end
    
    -- Check if hit by enemy
    if HasEnemyOn(other) then
        local enemy = GetEnemyFrom(other)
        if enemy then
            TakeDamage(enemy.mAttackDamage)
        end
    end
end

function OnTriggerEnter(other)
    -- Can be extended for health pickups, mana pickups, checkpoints, etc.
    if HasProjectileOn(other) then
        local proj = GetProjectileFrom(other)
        if proj then
            TakeDamage(proj.mStats.damage)
        end
    end
end

-- Helper function to handle damage
function TakeDamage(damage)
    local player = GetPlayer()
    if not player then return end
    
    -- Check invulnerability
    if player.isInvulnerable then return end
    
    -- Calculate actual damage after defense
    local actualDamage = math.max(1, damage - player.mDefense)
    player.mHealth = player.mHealth - actualDamage
    
    Log("Player took " .. tostring(actualDamage) .. " damage! Health: " .. tostring(player.mHealth))
    
    -- Transition to hurt state if still alive
    if player.mHealth > 0 then
        ChangeState(EntityID, "PlayerHurt")
    end
end

function updateTimers(dt)
    local player = GetPlayer()
    if not player then return end

    if player.isInvulnerable then 
        if player.mInvulnerabilityDuration > 0 then
            player.mInvulnerabilityDuration = player.mInvulnerabilityDuration - dt
        end
        if player.mInvulnerabilityDuration <= 0 then
            player.isInvulnerable = false
        end
    end

    if player.mDashCD > 0 then
        player.mDashCD = player.mDashCD - dt;
    end;
end

function InitializeAttackStats(player)
    if not player then return end

    Log("atkstats cnt:" .. GetAttackStatsCount(player))
    
    -- Clear existing attack stats
    local attackStats = player.attackStats
    
    -- Only initialize if empty (don't override serialized data)
    if attackStats and #attackStats > 0 then
        Log("Attack stats already initialized, skipping...")
        return
    end
    
    -- Create Attack 1 (Basic Combo - First Hit)
    local attack1 = CreateAttackStats()
    if attack1 then
        attack1.attackName = "Attack_1"
        attack1.animationClipName = "attack_1"
        attack1.mDamageMultiplier = attack1_damage
        attack1.mAttackSpeedMultiplier = attack1_speed
        attack1.triggerColliderIndex = 0
        attack1.elementType = ElementType.None
        attack1.manaCost = 0
        attack1.attackRange = attack1_range
        attack1.attackArc = attack1_arc
        attack1.applyBurn = false
        attack1.applyStun = false
        attack1.effectDuration = 0.0
        attack1.attackCd = 0.0
        attack1.attackCdCurr = 0.0
        attack1.attackIsInCoolDown = false
        
        AddAttackStats(player, attack1)
        Log("Added Attack_1 stats")
    end
    
    -- Create Attack 2 (Basic Combo - Second Hit)
    local attack2 = CreateAttackStats()
    if attack2 then
        attack2.attackName = "Attack_2"
        attack2.animationClipName = "attack_2"
        attack2.mDamageMultiplier = attack2_damage
        attack2.mAttackSpeedMultiplier = attack2_speed
        attack2.triggerColliderIndex = 1
        attack2.elementType = ElementType.None
        attack2.manaCost = 0
        attack2.attackRange = attack2_range
        attack2.attackArc = attack2_arc
        attack2.applyBurn = false
        attack2.applyStun = false
        attack2.effectDuration = 0.0
        attack2.attackCd = 0.0
        attack2.attackCdCurr = 0.0
        attack2.attackIsInCoolDown = false
        
        AddAttackStats(player, attack2)
        Log("Added Attack_2 stats")
    end
    
    -- Create Fire Slash (Elemental - Fire)
    local fireSlash = CreateAttackStats()
    if fireSlash then
        fireSlash.attackName = "Fire_Slash"
        fireSlash.animationClipName = "fire_slash"
        fireSlash.mDamageMultiplier = fireSlash_damage
        fireSlash.mAttackSpeedMultiplier = fireSlash_speed
        fireSlash.triggerColliderIndex = 2
        fireSlash.elementType = ElementType.Fire
        fireSlash.manaCost = math.floor(fireSlash_manaCost)
        fireSlash.attackRange = fireSlash_range
        fireSlash.attackArc = 120.0
        fireSlash.applyBurn = true
        fireSlash.applyStun = false
        fireSlash.effectDuration = fireSlash_burnDuration
        fireSlash.attackCd = 0.0
        fireSlash.attackCdCurr = 0.0
        fireSlash.attackIsInCoolDown = false
        
        AddAttackStats(player, fireSlash)
        Log("Added Fire_Slash stats")
    end
    
    -- Create Water Slash (Elemental - Water)
    local waterSlash = CreateAttackStats()
    if waterSlash then
        waterSlash.attackName = "Water_Slash"
        waterSlash.animationClipName = "water_slash"
        waterSlash.mDamageMultiplier = waterSlash_damage
        waterSlash.mAttackSpeedMultiplier = waterSlash_speed
        waterSlash.triggerColliderIndex = 3
        waterSlash.elementType = ElementType.Water
        waterSlash.manaCost = math.floor(waterSlash_manaCost)
        waterSlash.attackRange = waterSlash_range
        waterSlash.attackArc = 100.0
        waterSlash.applyBurn = false
        waterSlash.applyStun = true
        waterSlash.effectDuration = waterSlash_stunDuration
        waterSlash.attackCd = 0.0
        waterSlash.attackCdCurr = 0.0
        waterSlash.attackIsInCoolDown = false
        
        AddAttackStats(player, waterSlash)
        Log("Added Water_Slash stats")
    end
    
    -- Create Steam Burst (Fusion - Steam)
    local steamBurst = CreateAttackStats()
    if steamBurst then
        steamBurst.attackName = "Steam_Burst"
        steamBurst.animationClipName = "steam_burst"
        steamBurst.mDamageMultiplier = steamBurst_damage
        steamBurst.mAttackSpeedMultiplier = steamBurst_speed
        steamBurst.triggerColliderIndex = 4
        steamBurst.elementType = ElementType.Steam
        steamBurst.manaCost = math.floor(steamBurst_manaCost)
        steamBurst.attackRange = steamBurst_range
        steamBurst.attackArc = 360.0  -- Full AoE
        steamBurst.applyBurn = true   -- Steam has both effects
        steamBurst.applyStun = true
        steamBurst.effectDuration = 1.0
        steamBurst.attackCd = 0.0
        steamBurst.attackCdCurr = 0.0
        steamBurst.attackIsInCoolDown = false
        
        AddAttackStats(player, steamBurst)
        Log("Added Steam_Burst stats")
    end

    Log("2atkstats cnt:" .. GetAttackStatsCount(player))

end