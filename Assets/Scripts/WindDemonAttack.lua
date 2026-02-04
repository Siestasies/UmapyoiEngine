--copy and paste template for the states
--exposed vars for variable you want that can be editied in editor
ExposedVars = {
    attackExitRange = 12.0,
    HoverSpd = 1.5,
    chargeTime = 2.0,
    meleeRange = 4.0,
    dashSpeed = 10.0
}

local AttackCD = 0.0
local ChargeCD = 0.0
local HoverTime = 0.0
local enemy = nil
local baseX = 0.0
local animator = nil
local transform = nil
local isAttacking = false

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
        baseX = transform.position.x
    else
        return
    end

    if HasAnimator() then
        animator = GetAnimator()
    end
    
end

--takes in entity id from C++ to use in case needed
--uses dt for time updates etc
function state_update(entity, dt)

    --change back to chase 
    local playerId = FindEntityWithComponent("Player")
    if playerId == -1 then return end

    local playerTransform = GetTransformFrom(playerId)
    local dir = Vec2(playerTransform.worldPosition.x - transform.worldPosition.x, playerTransform.worldPosition.y - transform.worldPosition.y)
    local angle = math.deg(math.atan(dir.y, dir.x))

    local dx = playerTransform.worldPosition.x - transform.worldPosition.x
    local dy = playerTransform.worldPosition.y - transform.worldPosition.y
    local distSq = dx * dx + dy * dy
    
    if distSq > ExposedVars.attackExitRange * ExposedVars.attackExitRange then
        ChangeState(entity, "FireDemonChase")
        return
    end

    AttackCD = math.max(0, AttackCD - dt)

    if AttackCD > 0 and not isAttacking then
        --hover(dt)
        if animator.animator:HasFinished() then
            animator.animator:Play("idle",false)
        end

    elseif animator and animator.animator:GetCurrentClip() == "charging_atk" then
        isAttacking = true
        if animator.animator:HasFinished() then

            if distSq > ExposedVars.meleeRange * ExposedVars.meleeRange then
                --ranged attack
                local prefab = SpawnPrefab("wind.prefab", Vec2(10000, 10000))
                local projectile = GetProjectileFrom(prefab)

                projectile.mStats.damage = enemy.mAttackDamage
                PlayEntitySound(entity, "fire_enemy_attack", false, 0.3);

                if angle < 0 then
                    angle = angle + 360
                end

                AddForce(prefab, Vec2(transform.worldPosition.x,transform.worldPosition.y), dir, projectile.mStats.speed, angle - 180)
                AttackCD = enemy and enemy.mAttackSpeed or 2.0

                --ranged attack animation
                animator.animator:Play("attack", false)
            else
                --melee attack
                local collider = GetCollider(entity)
                if collider and collider.shapes:size() >= 3 then
                    collider.shapes[3].isActive = true  --Enable damage collider
                end

                --get normalised vector
                local length = math.sqrt(distSq)
                if length > 0 then
                    local normalisedDir = Vec2(dir.x/length, dir.y/length)
                    GetRigidBody().velocity = Vec2(normalisedDir.x * ExposedVars.dashSpeed, normalisedDir.y * ExposedVars.dashSpeed)
                end

                --melee animation
                animator.animator:Play("dash",false)
            end
        end
        isAttacking = false
    else
        isAttacking = false
        if animator and animator.animator:GetCurrentClip() ~= "charging_atk" then
            animator.animator:Play("charging_atk", false)
        end
    end


    if animator and animator.animator:GetCurrentClip() == "dash" and animator.animator:HasFinished() then
        local collider = GetCollider(entity)
        if collider and collider.shapes:size() >= 3 then
            collider.shapes[3].isActive = false  --Enable damage collider
        end

        GetRigidBody().velocity = Vec2(0,0)
        animator.animator:Play("idle", false)

        AttackCD = enemy and enemy.mAttackSpeed or 2.0
    end
end

--takes in entity id from C++ to use in case needed
function state_exit(entity)
    
end

function hover(dt)
    if HasTransform() then
        if transform then
            HoverTime = HoverTime + dt
            
            local hoverOffset = math.sin(HoverTime * 2.0) * ExposedVars.HoverSpd
            transform.worldPosition.x = baseX + hoverOffset
        end
    end
end
