--copy and paste template for the states
--exposed vars for variable you want that can be editied in editor
ExposedVars = {
    attackRange = 5.0,
    HoverSpd = 1.5,
    chargeTime = 2.0
}

local AttackCD = 0.0
local ChargeCD = 0.0
local HoverTime = 0.0
local enemy = nil
local baseX = 0.0
local transform = nil

--takes in entity id from C++ to use in case needed
function state_enter(entity)
    if HasEnemy() then
        enemy = GetEnemy()
        AttackCD = enemy.mAttackSpeed
    end

    ChargeCD = 0.0
    HoverTime = 0.0

    if HasTransform() then
        transform = GetTransform()
    else
        return
    end
    baseX = transform.position.x
end

--takes in entity id from C++ to use in case needed
--uses dt for time updates etc
function state_update(entity, dt)

    --change back to chase 
    local playerId = FindEntityWithComponent("Player")
    if playerId == -1 then return end

    local playerTransform = GetTransformFrom(playerId)
    local dir = Vec2(playerTransform.worldPosition.x - transform.worldPosition.x, playerTransform.worldPosition.y - transform.worldPosition.y)
    local angle = math.deg(math.atan2(dir.y, dir.x))
    
    local distSq = DistanceSquared(entity, playerId)
    if distSq > ExposedVars.attackRange * ExposedVars.attackRange then
        ChangeState(entity, "FireDemonChase")
        return
    end

    AttackCD = math.max(0, AttackCD - dt)
    --attack if no cd
    if ChargeCD > 0 then
        ChargeCD = math.max(0, ChargeCD - dt)
        if ChargeCD <= 0 then
            local prefab = SpawnPrefab("fireball.prefab", Vec2(10000, 10000))
            local projectile = GetProjectileFrom(prefab)

            projectile.mStats.damage = enemy.mAttackDamage
            PlayEntitySound(entity, "fire_enemy_attack", false, 0.3);

            if angle < 0 then
                angle = angle + 360
            end

            AddForce(prefab, Vec2(transform.worldPosition.x,transform.worldPosition.y), dir, projectile.mStats.speed, angle - 180)
            AttackCD = enemy and enemy.mAttackSpeed or 2.0
        end
    elseif AttackCD > 0 then
        --hover while attack is on cooldown
        hover(dt)
    else
        ChargeCD = ExposedVars.chargeTime
        if HasAnimator() then
            GetAnimator().animator:Play("FireCharge", true)
        end
    end

end

--takes in entity id from C++ to use in case needed
function state_exit(entity)
    
end

function hover(dt)
    if HasTransform() then
        local transform = GetTransform()
        if transform then
            HoverTime = HoverTime + dt
            
            local hoverOffset = math.sin(HoverTime * 2.0) * ExposedVars.HoverSpd
            transform.position.x = baseX + hoverOffset
        end
    end
end
