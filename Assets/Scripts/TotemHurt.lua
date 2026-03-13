ExposedVars = {
    totemElement = 1,
    initialDelay = 2.0,
    enemyHurtEffectDuration = 0.5
}

local enemy
local animator = nil
local playerId
local isDead
local enemyHurtEffectTimer
local isHurt = false
local isEffective = false
local isFusion = false
--local audio
local placeholderSprite = nil
local initialDelayTimer = 0.0

function Start()
    if HasEnemy() then
        enemy = GetEnemy()
    end
    playerId = FindEntityWithComponent("Player")
    if playerId == -1 or not IsEntityValid(playerId) then
        return
    end
    if HasAnimator() then
        animator = GetAnimator()
    end
    local childId = GetChildren(EntityID, 0)
    if IsEntityValid(childId) and HasSpriteOn(childId) then
        placeholderSprite = GetSpriteFrom(childId)
        placeholderSprite.alpha = 0
        if totemElement == 1 then
            placeholderSprite.spriteCell.y = 2.0
        elseif totemElement == 2 then
            placeholderSprite.spriteCell.y = 0.0
        elseif totemElement == 3 then
            placeholderSprite.spriteCell.y = 1.0
        end
    end
    initialDelayTimer = initialDelay
    enemyHurtEffectTimer = enemyHurtEffectDuration
end

function Update(dt)
    if animator and animator.animator:HasFinished() and initialDelayTimer <= 0 then
        if HasSprite() then
            local sprite = GetSprite()
            sprite.alpha = 0.0
        end
    else
        initialDelayTimer = initialDelayTimer - dt
    end
    if enemy then
        if enemy.mHealth <= 0  and not isDead then
            isDead = true
        end
        
        if placeholderSprite then
            placeholderSprite.alpha = 1
            if enemy.mHealth <= 0 then
                -- dead
            elseif enemy.mHealth <= 30 then
                placeholderSprite.spriteCell.x = 0.0
            elseif enemy.mHealth <= 60 then
                placeholderSprite.spriteCell.x = 1.0
            else
                placeholderSprite.spriteCell.x = 2.0
            end
        end

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
                enemyHurtEffectTimer = enemyHurtEffectDuration
                isHurt = false

                if HasSprite() then
                    local spriteComp = GetSprite()
                    spriteComp.tintColor = Vec3(1.0, 1.0, 1.0)
                end

            end
        end
    end
end

function OnDestroy()
    Log("Totem destroyed")
end

function OnTriggerEnter(other, triggerOwner)
    if totemElement == 1 then
        HandleDamageIfWind(triggerOwner)
    elseif totemElement == 2 then
        HandleDamageIfFire(triggerOwner)
    elseif totemElement == 3 then
        HandleDamageIfWater(triggerOwner)
    end
    --Log("totem is hit")
end

function OnTriggerExit(other)
    
end

function OnHurt(player, damage)
    -- damage handling logic here
    
    --audio = GetAudioComponent()
    --audio:play(EntityID, "WindDemonDamage")
    
    isHurt = true

    if enemy.mHealth <= 0 then
        return
    end 
    
    local transform = GetTransformFrom(EntityID)
    if isEffective and isFusion then
        enemy.mHealth = math.floor(enemy.mHealth - (damage - enemy.mDefense))
        SpawnFeedback(transform.worldPosition.x, transform.worldPosition.y, tostring(damage - enemy.mDefense), "crit")
    elseif isEffective then
        enemy.mHealth = math.floor(enemy.mHealth - (damage - enemy.mDefense))
        SpawnFeedback(transform.worldPosition.x, transform.worldPosition.y, tostring(damage - enemy.mDefense), "affinity")
    else
        enemy.mHealth = math.floor(enemy.mHealth - ((damage * 0.3) - enemy.mDefense))
        SpawnFeedback(transform.worldPosition.x, transform.worldPosition.y, tostring(damage - enemy.mDefense))
    end
end

function HandleDamageIfWind(trigger)
    if playerId == trigger then
        local playerComp = GetPlayerFrom(playerId)
        if playerComp then
            local attack = playerComp.attackStats[math.floor(playerComp.currAttackIndex + 1)]

            if attack.elementType == ElementType.Pyronado or
               attack.elementType == ElementType.Steam then
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
    elseif HasProjectileOn(trigger) then
        local playerComp = GetPlayerFrom(playerId)
        local projectile = GetProjectileFrom(trigger)
        if playerComp then
            local attack = playerComp.attackStats[math.floor(playerComp.currAttackIndex + 1)]

            if attack.elementType == ElementType.Pyronado or
               attack.elementType == ElementType.Steam then
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

function HandleDamageIfFire(trigger)
    if playerId == trigger then
        local playerComp = GetPlayerFrom(playerId)
        if playerComp then
            local attack = playerComp.attackStats[math.floor(playerComp.currAttackIndex + 1)]

            if attack.elementType == ElementType.Whirlpool or
               attack.elementType == ElementType.Steam then
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
        end
    elseif HasProjectileOn(trigger) then
        local playerComp = GetPlayerFrom(playerId)
        local projectile = GetProjectileFrom(trigger)
        if playerComp then
            local attack = playerComp.attackStats[math.floor(playerComp.currAttackIndex + 1)]

            if attack.elementType == ElementType.Whirlpool or
               attack.elementType == ElementType.Steam then
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
        end
    end
end

function HandleDamageIfWater(trigger)
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
        local projectile = GetProjectileFrom(trigger)
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