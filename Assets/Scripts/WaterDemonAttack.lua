--copy and paste template for the states
--exposed vars for variable you want that can be editied in editor
ExposedVars = {
    attackRange = 5.0,
    HoverSpd = 1.5,
    damageDuration = 1.0,
    chargeTime = 2.0
}

local AttackCD = 0.0
local ChargeCD = 0.0
local damageTimer = 0.0
local HoverTime = 0.0
local enemy = nil
local baseX = 0.0

--takes in entity id from C++ to use in case needed
function state_enter(entity)
    if HasEnemy() then
        enemy = GetEnemy()
        AttackCD = enemy.mAttackSpeed
    end

    ChargeCD = 0.0
    HoverTime = 0.0
    damageTimer = 0.0

    local transform = GetTransform()
    baseX = transform.position.x
end

--takes in entity id from C++ to use in case needed
--uses dt for time updates etc
function state_update(entity, dt)

    --change back to chase 
    local playerId = FindEntityWithComponent("Player")
    if playerId == -1 then return end
    
    local distSq = DistanceSquared(entity, playerId)
    if distSq > ExposedVars.attackRange * ExposedVars.attackRange then
        ChangeState(entity, "WaterDemonChase")
        return
    end

    if damageTimer > 0 then
        damageTimer = damageTimer - dt
        if damageTimer <= 0 and HasCollider() then
            local collider = GetCollider(entity)
            if collider and #collider.shapes >= 3 then
                collider.shapes[2].enabled = false  -- Disable damage collider
            end
        end
    end

    AttackCD = math.max(0, AttackCD - dt)
    --attack if no cd
    if ChargeCD > 0 then
        ChargeCD = math.max(0, ChargeCD - dt)
        if ChargeCD <= 0 then
            -- Enable damage collider
            if HasCollider() then
                local collider = GetCollider(entity)
                if collider and #collider.shapes >= 3 then
                    collider.shapes[2].enabled = true
                    damageTimer = ExposedVars.damageDuration
                end
            end
            AttackCD = enemy and enemy.mAttackSpeed or 2.0
        end
    elseif AttackCD > 0 then
        --hover while attack is on cooldown
        hover(dt)
    else
        ChargeCD = ExposedVars.chargeTime
        if HasAnimator() then
            GetAnimator().animator:Play("WaterCharge", true)
        end
    end

end

--takes in entity id from C++ to use in case needed
function state_exit(entity)
    if HasCollider() then
        local collider = GetCollider(entity)
        if collider and #collider.shapes >= 3 then
            collider.shapes[2].enabled = false
        end
    end
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
