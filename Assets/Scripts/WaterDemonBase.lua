local audio = nil
ExposedVars = {
    --empty for now
    enemyHurtEffectDuration = 0.5;
}

local enemy
local playerId
local isDead
local enemyHurtEffectTimer
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

    isEffective = false
    isFusion = false
    isHurt = false
    enemyHurtEffectTimer = enemyHurtEffectDuration

    ChangeState(EntityID, "WaterDemonIdle")
end

function Update(dt)
    if enemy then
        if enemy.mHealth <= 0  and not isDead then
            isDead = true
            ChangeState(EntityID, "WaterDemonSuicide")
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
    
end

--optional use as needed
function OnCollisionEnter(other)
    
end

function OnCollisionExit(other)

end

function HandleCollision(trigger)

   
    if playerId == trigger then
        local playerComp = GetPlayerFrom(playerId)
        if playerComp then

            local attack = playerComp.attackStats[math.floor(playerComp.currAttackIndex + 1)]

            --Log("=== ATTACK DEBUG ===")
            --Log("Attack Index: " .. tostring(playerComp.currAttackIndex))
            --Log("Attack Name: " .. tostring(attack.attackName))
            --Log("Element Type (raw): " .. tostring(attack.elementType))
            --Log("ElementType.None: " .. tostring(ElementType.None))
            --Log("ElementType.Fire: " .. tostring(ElementType.Fire))
            --Log("ElementType.Water: " .. tostring(ElementType.Water))
            --Log("ElementType.Wind: " .. tostring(ElementType.Wind))
            --Log("ElementType.Steam: " .. tostring(ElementType.Steam))
            --Log("===================")
--

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

            local transform = GetTransform()
            if transform then
                --PlayOneShotAtEntity(EntityID, "hurt", 0.5)
            end
        end
    end
end

function OnHurt(player, damage)
    -- damage handling logic here
    
        PlayEntitySound(EntityID, "enemy_hurt", false, 0.8);
        PlayEntitySound(EntityID, "enemy_hit", false, 0.8);

    enemy.mHealth = enemy.mHealth - (damage - enemy.mDefense)

    isHurt = true

    local transform = GetTransformFrom(EntityID)
    if isEffective and isFusion then
        ChangeState(EntityID, "WaterDemonStunned")
        isEffective = false
        isFusion = false
        SpawnFeedback(transform.worldPosition.x, transform.worldPosition.y, tostring(damage - enemy.mDefense), "crit")
    elseif isEffective then
        SpawnFeedback(transform.worldPosition.x, transform.worldPosition.y, tostring(damage - enemy.mDefense), "affinity")
    else
        SpawnFeedback(transform.worldPosition.x, transform.worldPosition.y, tostring(damage - enemy.mDefense))
    end
    
    audio = GetAudioComponent()
    audio:play(EntityID, "Water Demon Damage Scream")
end

function OnTriggerEnter(other, triggerOwner)
    HandleCollision(triggerOwner)
    --Log("enemy is hit")

end

function OnTriggerExit(other)
    
end
