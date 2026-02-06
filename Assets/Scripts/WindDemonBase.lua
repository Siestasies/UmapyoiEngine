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
local audio

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

    ChangeState(EntityID, "WindDemonIdle")
end

function Update(dt)
    if enemy then
        if enemy.mHealth <= 0  and not isDead then
            isDead = true
            ChangeState(EntityID, "WindDemonDead")
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
--
            if attack.elementType == ElementType.Steam or  
            attack.elementType == ElementType.Whirlpool then 
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

            local transform = GetTransform()
            if transform then
                
            end
        end
    end
end

function OnHurt(player, damage)
    -- damage handling logic here
    
    audio = GetAudioComponent()
    audio:play(EntityID, "WindDemonDamage")

    enemy.mHealth = enemy.mHealth - (damage - enemy.mDefense)

    isHurt = true

    if isEffective and isFusion then
        ChangeState(EntityID, "WindDemonStunned")
    end
end

function OnTriggerEnter(other, triggerOwner)
    HandleCollision(triggerOwner)
    --Log("enemy is hit")

end

function OnTriggerExit(other)
    
end
