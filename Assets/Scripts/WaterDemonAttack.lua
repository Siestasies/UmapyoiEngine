--copy and paste template for the states
--exposed vars for variable you want that can be editied in editor
ExposedVars = {
    attackExitRange = 30.0,
    HoverSpd = 1.5,
    damageDuration = 1.0,
    chargeTime = 0.54
}

local AttackCD = 0.0
local ChargeCD = 0.0
local damageTimer = 0.0
local HoverTime = 0.0
local enemy = nil
local baseX = 0.0
local animator = nil
local vfxAnimator = nil;

--takes in entity id from C++ to use in case needed
function state_enter(entity)
    if HasEnemy() then
        enemy = GetEnemy()
        AttackCD = enemy.mAttackSpeed
    end

    ChargeCD = ExposedVars.chargeTime
    HoverTime = 0.0
    damageTimer = 0.0

    local transform = GetTransform()
    baseX = transform.position.x
    GetRigidBody().velocity = Vec2(0.0, 0.0)
    GetPathFinding().enabled = false;

    if HasEnemy() then
        enemy = GetEnemy()
    else
        Log("not enemy")
        return
    end

    if HasAnimator() then
        animator = GetAnimator()
    end

    if HasChildren(EntityID, 0) then
        local vfxID = GetChildren(EntityID, 0)

        if HasAnimatorOn(vfxID) then
            vfxAnimator = GetAnimatorFrom(vfxID)
        end

    end

    animator.animator:Play("charging_atk", false)
end

--takes in entity id from C++ to use in case needed
--uses dt for time updates etc
function state_update(entity, dt)

    --change back to chase 
    local playerId = FindEntityWithComponent("Player")
    if playerId == -1 then return end

    local playerTransform = GetTransformFrom(playerId)
    local myTransform = GetTransformFrom(entity)
    if not playerTransform or not myTransform then
        return
    end
    
    local dx = playerTransform.worldPosition.x - myTransform.worldPosition.x
    local dy = playerTransform.worldPosition.y - myTransform.worldPosition.y
    local distSq = dx * dx + dy * dy

    if distSq > ExposedVars.attackExitRange * ExposedVars.attackExitRange then
        ChangeState(entity, "WaterDemonChase")
        return
    end
    --Log("Update Attack player world pos" .. playerTransform.worldPosition.x .. ", " .. playerTransform.worldPosition.y)
    --Log("Update Attack " .. distSq .. " " .. "attackrange " .. enemy.mAttackRange * enemy.mAttackRange)

    if damageTimer > 0 then
        damageTimer = damageTimer - dt
        if damageTimer <= 0 and HasCollider() then
            local collider = GetCollider(entity)
            if collider and collider.shapes:size() >= 3 then
                collider.shapes[3].isActive = false  -- Disable damage collider
            end
        end
    end

    AttackCD = math.max(0, AttackCD - dt)
    --attack if no cd
    if ChargeCD > 0 then
        ChargeCD = math.max(0, ChargeCD - dt)

        if animator.animator:GetCurrentClip() ~= "charging_atk" then 
            animator.animator:Play("charging_atk", false)
        end

        if ChargeCD <= 0 then
            -- Enable damage collider
            if HasCollider() then
                local collider = GetCollider(entity)
                if collider and collider.shapes:size() >= 3 then
                    animator.animator:Play("atk", false)

                    if vfxAnimator ~= nil then
                        vfxAnimator.animator:Play("splash", true)
                    end

                    collider.shapes[3].isActive = true
                    damageTimer = ExposedVars.damageDuration
                end
            end
            AttackCD = enemy and enemy.mAttackSpeed or 2.0
        end
    elseif AttackCD > 0 then
        --hover while attack is on cooldown
        --hover(dt)

        if animator.animator:HasFinished() then
            animator.animator:Play("idle", false)
        end
        
    else
        ChargeCD = ExposedVars.chargeTime
        if HasAnimator() then
            --GetAnimator().animator:Play("WaterCharge", true)
        end
    end

end

--takes in entity id from C++ to use in case needed
function state_exit(entity)

    GetPathFinding().enabled = true;

    if HasCollider() then
        local collider = GetCollider(entity)
        if collider and collider.shapes:size() >= 3 then
            collider.shapes[3].isActive = false
        end
    end
end

function hover(dt)
    if HasTransform() then
       local transform = GetTransform()
       if transform then
           HoverTime = HoverTime + dt
           
           local hoverOffset = math.sin(HoverTime * 2.0) * ExposedVars.HoverSpd
           transform.worldPosition.x = baseX + hoverOffset
       end
    end
end
