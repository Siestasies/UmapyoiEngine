-- PlayerBase.lua
-- Base script for player entity - handles health, mana regen, and death transitions

ExposedVars = {
    debugMode = false,
    
    -- Basic Attack 1 settings
    attack1_damage = 1.0,
    attack1_speed = 1.0,
    
    -- Basic Attack 2 settings
    attack2_damage = 1.2,
    attack2_speed = 1.0,
    
    -- Fire Slash settings
    fireSlash_damage = 1.5,
    fireSlash_speed = 1.0,
    fireSlash_manaCost = 20,
    fireSlash_burnDuration = 3.0,
    
    -- Water Slash settings
    waterSlash_damage = 1.3,
    waterSlash_speed = 1.0,
    waterSlash_manaCost = 20,
    waterSlash_stunDuration = 1.5,

    -- Wind Dash settings
    windDash_damage = 1.3,
    windDash_speed = 1.0,
    windDash_manaCost = 5,
    
    -- Steam Burst settings
    steamBurst_damage = 2.5,
    steamBurst_speed = 1.0,
    steamBurst_manaCost = 30,

    -- Pyronado settings
    pyronado_damage = 2.5,
    pyronado_speed = 1.0,
    pyronado_manaCost = 30,

    -- Whirlpool settings
    whirlpool_damage = 2.5,
    whirlpool_speed = 1.0,
    whirlpool_manaCost = 30
}

local isDead

function Start()
    if HasPlayer() then
        local player = GetPlayer()
        if player then
            Log("PlayerBase initialized - Health: " .. tostring(player.mHealth) .. "/" .. tostring(player.mMaxHealth))
            InitializeAttackStats(player)
            ChangeState(EntityID, "PlayerIdle")
            isDead = false
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
    if player.mHealth <= 0 and isDead == false then
        isDead = true
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

function OnTriggerEnter(other, triggerOwner)
    -- Can be extended for health pickups, mana pickups, checkpoints, etc.
    if HasProjectileOn(triggerOwner) then
        local proj = GetProjectileFrom(triggerOwner)
        if proj then
            TakeDamage(proj.mStats.damage)
        end
    end

    if HasEnemyOn(triggerOwner) then
        local enemy = GetEnemyFrom(triggerOwner)
        if enemy then
            TakeDamage(enemy.mAttackDamage)
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
    ClearAttackStats(player)
    
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
        attack1.triggerColliderIndex = 1
        attack1.elementType = ElementType.None
        attack1.manaCost = 0
        attack1.attackRange = 0.0
        attack1.attackArc = 0.0
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
        attack2.triggerColliderIndex = 2
        attack2.elementType = ElementType.None
        attack2.manaCost = 0
        attack2.attackRange = 0.0
        attack2.attackArc = 0.0
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
        fireSlash.triggerColliderIndex = 3
        fireSlash.elementType = ElementType.Fire
        fireSlash.manaCost = math.floor(fireSlash_manaCost)
        fireSlash.attackRange = 0.0
        fireSlash.attackArc = 0.0
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
        waterSlash.triggerColliderIndex = 4
        waterSlash.elementType = ElementType.Water
        waterSlash.manaCost = math.floor(waterSlash_manaCost)
        waterSlash.attackRange = waterSlash_range
        waterSlash.attackArc = 0.0
        waterSlash.applyBurn = false
        waterSlash.applyStun = true
        waterSlash.effectDuration = waterSlash_stunDuration
        waterSlash.attackCd = 0.0
        waterSlash.attackCdCurr = 0.0
        waterSlash.attackIsInCoolDown = false
        
        AddAttackStats(player, waterSlash)
        Log("Added Water_Slash stats")
    end

    -- Create Wind Dash (Elemental - Wind)
    local windDash = CreateAttackStats()
    if windDash then
        windDash.attackName = "Wind_Dash"
        windDash.animationClipName = "wind_dash"
        windDash.mDamageMultiplier = windDash_damage
        windDash.mAttackSpeedMultiplier = windDash_speed
        windDash.triggerColliderIndex = 5
        windDash.elementType = ElementType.Wind
        windDash.manaCost = math.floor(windDash_manaCost)
        windDash.attackRange = 0.0
        windDash.attackArc = 0.0
        windDash.applyBurn = false
        windDash.applyStun = true
        windDash.effectDuration = 0.0
        windDash.attackCd = 2.0
        windDash.attackCdCurr = 0.0
        windDash.attackIsInCoolDown = false
        
        AddAttackStats(player, windDash)
        Log("Added Wind_Dash stats")
    end
    
    -- Create Steam Burst (Fusion - Steam)
    local steamBurst = CreateAttackStats()
    if steamBurst then
        steamBurst.attackName = "Steam_Burst"
        steamBurst.animationClipName = "steam_burst"
        steamBurst.mDamageMultiplier = steamBurst_damage
        steamBurst.mAttackSpeedMultiplier = steamBurst_speed
        steamBurst.triggerColliderIndex = 6
        steamBurst.elementType = ElementType.Steam
        steamBurst.manaCost = math.floor(steamBurst_manaCost)
        steamBurst.attackRange = 0.0
        steamBurst.attackArc = 0.0 
        steamBurst.applyBurn = true  
        steamBurst.applyStun = true
        steamBurst.effectDuration = 1.0
        steamBurst.attackCd = 0.0
        steamBurst.attackCdCurr = 0.0
        steamBurst.attackIsInCoolDown = false
        
        AddAttackStats(player, steamBurst)
        Log("Added Steam_Burst stats")
    end

    -- Create Steam Burst (Fusion - Pyronado)
    local pyronado = CreateAttackStats()
    if pyronado then
        pyronado.attackName = "Pyronado"
        pyronado.animationClipName = "pyronado"
        pyronado.mDamageMultiplier = pyronado_damage
        pyronado.mAttackSpeedMultiplier = pyronado_speed
        pyronado.triggerColliderIndex = 7
        pyronado.elementType = ElementType.Pyronado
        pyronado.manaCost = math.floor(pyronado_manaCost)
        pyronado.attackRange = 0.0
        pyronado.attackArc = 0.0  
        pyronado.applyBurn = true   
        pyronado.applyStun = true
        pyronado.effectDuration = 1.0
        pyronado.attackCd = 0.0
        pyronado.attackCdCurr = 0.0
        pyronado.attackIsInCoolDown = false
        
        AddAttackStats(player, pyronado)
        Log("Added Pyronado stats")
    end

    -- Create Steam Burst (Fusion - Whirlpool)
    local whirlpool = CreateAttackStats()
    if whirlpool then
        whirlpool.attackName = "Whirlpool"
        whirlpool.animationClipName = "whirlpool"
        whirlpool.mDamageMultiplier = whirlpool_damage
        whirlpool.mAttackSpeedMultiplier = whirlpool_speed
        whirlpool.triggerColliderIndex = 8
        whirlpool.elementType = ElementType.Whirlpool
        whirlpool.manaCost = math.floor(whirlpool_manaCost)
        whirlpool.attackRange = 0.0
        whirlpool.attackArc = 0.0  
        whirlpool.applyBurn = true   
        whirlpool.applyStun = true
        whirlpool.effectDuration = 1.0
        whirlpool.attackCd = 0.0
        whirlpool.attackCdCurr = 0.0
        whirlpool.attackIsInCoolDown = false
        
        AddAttackStats(player, whirlpool)
        Log("Added Whirlpool stats")
    end

    Log("atkstats cnt:" .. GetAttackStatsCount(player))

end