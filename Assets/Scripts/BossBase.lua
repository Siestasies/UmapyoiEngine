-- BossBase: Runs on the boss entity alongside whichever state is active
-- Fully self-contained. No global functions or shared data.
-- States are responsible for their own spawning, tracking, and transitions.
-- Base only handles: boss HP death check, hurt flash, and collision/damage.

local audio = nil
local bhp = require("BossHPState")
ExposedVars = {
    enemyHurtEffectDuration = 0.5,
    bossMaxHealth = 500
}

local enemy
local playerId
local isDead = false
local enemyHurtEffectTimer = 0
local isHurt = false
local isEffective = false
local isFusion = false

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

    -- Boss starts hidden during phase 1
    if HasSprite() then
        local spriteComp = GetSprite()
        spriteComp.alpha = 0
    end

    -- Disable boss colliders during phase 1
    if HasCollider() then
        local collider = GetCollider(EntityID)
        if collider then
            for i = 1, collider.shapes:size() do
                collider.shapes[i].isActive = false
            end
        end
    end

    ChangeState(EntityID, "BossPhase1")
    bhp.SetBossHP(enemy.mHealth)
end

function Update(dt)
    if isDead then return end

    -- Boss death check: only when boss is visible (phases 2 and 3)
    -- We check sprite alpha to know if the boss is "active" without needing phase state
    if enemy and enemy.mHealth <= 0 then
        local spriteComp = nil
        if HasSprite() then
            spriteComp = GetSprite()
        end

        -- Only die if boss is actually revealed (alpha > 0)
        if spriteComp and spriteComp.alpha > 0 then
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

-- ============ COLLISION / DAMAGE ============
-- Only processes hits when boss is visible (alpha > 0)

function HandleCollision(trigger)
    if HasSprite() then
        local spriteComp = GetSprite()
        if spriteComp.alpha <= 0 then return end
    else
        return
    end

    enemy = GetEnemy()
    if not enemy or enemy.mHealth <= 0 then
        return
    end

    if playerId == trigger then
        local playerComp = GetPlayerFrom(playerId)
        if playerComp then
            local attack = playerComp.attackStats[math.floor(playerComp.currAttackIndex + 1)]

            if bhp.GetBossElement() == 0 then -- water
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
            elseif bhp.GetBossElement() == 1 then -- fire
                if attack.elementType == ElementType.Steam or
                    attack.elementType == ElementType.Whirlpool then
                    isEffective = true
                    isFusion = true
                    OnHurt(playerId, math.floor(playerComp.mAttackDamage))
                elseif attack.elementType == ElementType.Water then
                    isEffective = true
                    isFusion = false
                    OnHurt(playerId, math.floor(playerComp.mAttackDamage))
                else
                    isEffective = false
                    isFusion = false
                    OnHurt(playerId, math.floor(playerComp.mAttackDamage * 0.3))
                end
            else -- wind
                if attack.elementType == ElementType.Steam or
                    attack.elementType == ElementType.Pyronado then
                    isEffective = true
                    isFusion = true
                    OnHurt(playerId, math.floor(playerComp.mAttackDamage))
                elseif attack.elementType == ElementType.Fire then
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
    elseif HasProjectileOn(trigger) then
        local playerComp = GetPlayerFrom(playerId)
        if playerComp then
            local attack = playerComp.attackStats[math.floor(playerComp.currAttackIndex + 1)]

            if bhp.currElement == 0 then -- water
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
            elseif bhp.currElement == 1 then -- fire
                if attack.elementType == ElementType.Steam or
                    attack.elementType == ElementType.Whirlpool then
                    isEffective = true
                    isFusion = true
                    OnHurt(playerId, math.floor(playerComp.mAttackDamage))
                elseif attack.elementType == ElementType.Water then
                    isEffective = true
                    isFusion = false
                    OnHurt(playerId, math.floor(playerComp.mAttackDamage))
                else
                    isEffective = false
                    isFusion = false
                    OnHurt(playerId, math.floor(playerComp.mAttackDamage * 0.3))
                end
            else -- wind
                if attack.elementType == ElementType.Steam or
                    attack.elementType == ElementType.Pyronado then
                    isEffective = true
                    isFusion = true
                    OnHurt(playerId, math.floor(playerComp.mAttackDamage))
                elseif attack.elementType == ElementType.Fire then
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
end

function OnHurt(player, damage)
    if not enemy then return end

    PlayEntitySound(EntityID, "enemy_hurt", false, 0.8)
    PlayEntitySound(EntityID, "enemy_hit", false, 0.8)

    enemy.mHealth = enemy.mHealth - (damage - enemy.mDefense)
    bhp.SetBossHP(enemy.mHealth)
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

    GetAudioComponent():play(EntityID, "SFX_Boss_Hurt2")
end

function OnTriggerEnter(other, triggerOwner)
    HandleCollision(triggerOwner)
end

function OnTriggerExit(other)
end
