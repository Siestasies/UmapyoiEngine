ExposedVars = {
    --empty for now
    enemyHurtEffectDuration = 0.5;
}

local enemy
local playerId
local isDead
local enemyHurtEffectTimer
local isHurt = false

function Start()
    if HasEnemy() then
        enemy = GetEnemy()
    end

    playerId = FindEntityWithComponent("Player")
    if playerId == -1 or not IsEntityValid(playerId) then
        return
    end

    enemyHurtEffectTimer = enemyHurtEffectDuration

    ChangeState(EntityID, "WindDemonIdle")
end

function Update(dt)
    if enemy then
        if enemy.mHealth < 0 and not isDead then
            isDead = true
            ChangeState(EntityID, "WindDemonDead")
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
            OnHurt(playerId, playerComp.mAttackDamage)

            local transform = GetTransform()
            if transform then
                --PlayOneShotAtEntity(EntityID, "hurt", 0.5)
            end
        end
    end
end

function OnHurt(player, damage)
    -- damage handling logic here

    enemy.mHealth = enemy.mHealth - (damage - enemy.mDefense)

    isHurt = true

    PlayEntitySound(EntityID, "enemy_hurt", false, 0.8);
    PlayEntitySound(EntityID, "enemy_hit", false, 0.8);
end

function OnTriggerEnter(other, triggerOwner)
    HandleCollision(triggerOwner)
end

function OnTriggerExit(other)
    
end