local audio = nil
ExposedVars = {
    enemyHurtEffectDuration = 0.5,
    phase2Trigger = 0,          -- all 3 elites dead triggers phase 2
    totalTotems = 3,
    bossMaxHealth = 500
}

local enemy
local playerId
local isDead = false
local enemyHurtEffectTimer = 0
local isHurt = false
local isEffective = false
local isFusion = false

-- Phase tracking
local currentPhase = 1          -- 1 = elite phase, 2 = bullet hell + totems, 3 = direct fight
local eliteIds = {}             -- populated from children
local elitesAlive = 3
local totemsAlive = 0
local totemIds = {}

-- Camera reference
local cameraId = -1

function Start()
    if HasEnemy() then
        enemy = GetEnemy()
    end

    playerId = FindEntityWithComponent("Player")
    if playerId == -1 or not IsEntityValid(playerId) then
        return
    end

    isDead = false
    isEffective = false
    isFusion = false
    isHurt = false
    enemyHurtEffectTimer = ExposedVars.enemyHurtEffectDuration
    currentPhase = 1
    elitesAlive = 3

    cameraId = FindEntityWithComponent("Camera")

    -- Get the 3 elite children (indices 0, 1, 2)
    -- Elites are parented under the boss in the hierarchy
    eliteIds = {}
    for i = 0, 2 do
        if HasChildren(EntityID, i) then
            local childId = GetChildren(EntityID, i)
            if IsEntityValid(childId) and HasEnemyOn(childId) then
                table.insert(eliteIds, childId)
            end
        end
    end

    elitesAlive = #eliteIds
    Log("Boss initialized with " .. tostring(elitesAlive) .. " elites")

    -- Boss starts inactive/hidden during phase 1
    if HasSprite() then
        local spriteComp = GetSprite()
        spriteComp.visible = false
    end

    -- Disable boss collider during phase 1
    if HasCollider() then
        local collider = GetCollider(EntityID)
        if collider then
            for i = 1, collider.shapes:size() do
                collider.shapes[i].isActive = false
            end
        end
    end

    ChangeState(EntityID, "BossPhase1")
end

function Update(dt)
    if isDead then return end

    -- Track elite deaths in phase 1
    if currentPhase == 1 then
        local alive = 0
        for i, eliteId in ipairs(eliteIds) do
            if IsEntityValid(eliteId) then
                local eliteEnemy = GetEnemyFrom(eliteId)
                if eliteEnemy and eliteEnemy.mHealth > 0 then
                    alive = alive + 1
                end
            end
        end

        if alive ~= elitesAlive then
            elitesAlive = alive
            Log("Elites remaining: " .. tostring(elitesAlive))
        end

        if elitesAlive <= 0 then
            Log("All elites defeated! Transitioning to combining phase...")
            currentPhase = 2
            ChangeState(EntityID, "BossCombine")
            return
        end
    end

    -- Track totem deaths in phase 2
    if currentPhase == 2 then
        local alive = 0
        for i, totemId in ipairs(totemIds) do
            if IsEntityValid(totemId) then
                local totemEnemy = GetEnemyFrom(totemId)
                if totemEnemy and totemEnemy.mHealth > 0 then
                    alive = alive + 1
                end
            end
        end

        if alive ~= totemsAlive then
            totemsAlive = alive
            Log("Totems remaining: " .. tostring(totemsAlive))
        end

        if totemsAlive <= 0 and totemsAlive ~= -1 then
            Log("All totems destroyed! Transitioning to phase 3...")
            currentPhase = 3
            ChangeState(EntityID, "BossPhase3")
            return
        end
    end

    -- Boss death check (phases 2 and 3)
    if currentPhase >= 2 and enemy then
        if enemy.mHealth <= 0 and not isDead then
            isDead = true
            ChangeState(EntityID, "BossDeath")
            return
        end
    end

    -- Hurt flash effect
    if isHurt then
        enemyHurtEffectTimer = enemyHurtEffectTimer - dt

        if HasSprite() then
            local spriteComp = GetSprite()
            if isEffective then
                spriteComp.tintColor = Vec3(1.0, 0.5, 0.5)
            else
                spriteComp.tintColor = Vec3(1.0, 1.0, 0.0)
            end
        end

        if enemyHurtEffectTimer <= 0.0 then
            enemyHurtEffectTimer = ExposedVars.enemyHurtEffectDuration
            isHurt = false

            if HasSprite() then
                local spriteComp = GetSprite()
                spriteComp.tintColor = Vec3(1.0, 1.0, 1.0)
            end
        end
    end
end

function OnDestroy()
end

-- ============ HELPER FUNCTIONS FOR STATES ============

-- Called by BossPhase2 to register spawned totems
function RegisterTotems(ids)
    totemIds = ids
    totemsAlive = #ids
    Log("Registered " .. tostring(totemsAlive) .. " totems")
end

function GetCurrentPhase()
    return currentPhase
end

function SetCurrentPhase(phase)
    currentPhase = phase
end

function GetPlayerId()
    return playerId
end

function GetEliteIds()
    return eliteIds
end

function GetCameraId()
    return cameraId
end

function GetTotemsAlive()
    return totemsAlive
end

-- ============ COLLISION / DAMAGE ============

function HandleCollision(trigger)
    -- Only take damage in phases 2 and 3
    if currentPhase < 2 then return end

    enemy = GetEnemy()
    if not enemy or enemy.mHealth <= 0 then
        return
    end

    if playerId == trigger then
        local playerComp = GetPlayerFrom(playerId)
        if playerComp then
            local attack = playerComp.attackStats[math.floor(playerComp.currAttackIndex + 1)]

            if attack.elementType == ElementType.Whirlpool or
               attack.elementType == ElementType.Pyronado then
                isEffective = true
                isFusion = true
                OnHurt(playerId, math.floor(playerComp.mAttackDamage))
            elseif attack.elementType == ElementType.Wind then
                isEffective = true
                isFusion = false
                OnHurt(playerId, math.floor(playerComp.mAttackDamage))
            else
                isEffective = false
                isFusion = false
                OnHurt(playerId, math.floor(playerComp.mAttackDamage * 0.3))
            end
        end
    elseif HasProjectileOn(trigger) then
        local playerComp = GetPlayerFrom(playerId)
        if playerComp then
            local attack = playerComp.attackStats[math.floor(playerComp.currAttackIndex + 1)]

            if attack.elementType == ElementType.Whirlpool or
               attack.elementType == ElementType.Pyronado then
                isEffective = true
                isFusion = true
                OnHurt(playerId, math.floor(playerComp.mAttackDamage))
            elseif attack.elementType == ElementType.Wind then
                isEffective = true
                isFusion = false
                OnHurt(playerId, math.floor(playerComp.mAttackDamage))
            else
                isEffective = false
                isFusion = false
                OnHurt(playerId, math.floor(playerComp.mAttackDamage * 0.3))
            end
        end
    end
end

function OnHurt(player, damage)
    if not enemy then return end

    PlayEntitySound(EntityID, "enemy_hurt", false, 0.8)
    PlayEntitySound(EntityID, "enemy_hit", false, 0.8)

    enemy.mHealth = enemy.mHealth - (damage - enemy.mDefense)
    isHurt = true

    local transform = GetTransformFrom(EntityID)
    if isEffective and isFusion then
        SpawnFeedback(transform.worldPosition.x, transform.worldPosition.y, tostring(damage - enemy.mDefense), "crit")
        isEffective = false
        isFusion = false
    elseif isEffective then
        SpawnFeedback(transform.worldPosition.x, transform.worldPosition.y, tostring(damage - enemy.mDefense), "affinity")
    else
        SpawnFeedback(transform.worldPosition.x, transform.worldPosition.y, tostring(damage - enemy.mDefense))
    end

    audio = GetAudioComponent()
    audio:play(EntityID, "Boss Damage")
end

function OnTriggerEnter(other, triggerOwner)
    HandleCollision(triggerOwner)
end

function OnTriggerExit(other)
end
