ExposedVars = {
    attackExitRange = 30.0,
    meleeRange = 15.0,
    chargeTime = 1.0,
    dashSpeed = 150.0,
    damageDuration = 0.5
}

local AttackCD = 0.0
local ChargeCD = 0.0
local damageTimer = 0.0
local enemy = nil
local animator = nil
local transform = nil
local spriteComp = nil
local vfxAnimator = nil
local isDashing = false
local dashDirX = 0
local dashDirY = 0

function state_enter(entity)
    if HasEnemy() then
        enemy = GetEnemy()
    else
        Log("not enemy")
        return
    end

    ChargeCD = ExposedVars.chargeTime
    AttackCD = 0.0
    damageTimer = 0.0
    isDashing = false

    GetRigidBody().velocity = Vec2(0.0, 0.0)
    GetPathFinding().reachedGoal = true

    if HasTransform() then
        transform = GetTransform()
    end

    if HasAnimator() then
        animator = GetAnimator()
    end

    if HasSprite() then
        spriteComp = GetSprite()
    end

    if HasChildren(EntityID, 0) then
        local vfxID = GetChildren(EntityID, 0)
        if HasAnimatorOn(vfxID) then
            vfxAnimator = GetAnimatorFrom(vfxID)
        end
    end

    animator.animator:Play("charging_atk", false)
end

function state_update(entity, dt)
    local playerId = FindEntityWithComponent("Player")
    if playerId == -1 then return end

    local playerTransform = GetTransformFrom(playerId)
    if not playerTransform or not transform then return end

    local dx = playerTransform.worldPosition.x - transform.worldPosition.x
    local dy = playerTransform.worldPosition.y - transform.worldPosition.y
    local distSq = dx * dx + dy * dy

    -- Exit to chase if player too far
    if distSq > ExposedVars.attackExitRange * ExposedVars.attackExitRange then
        ChangeState(entity, "WindDemonChase")
        return
    end

    -- Handle damage collider timer
    if damageTimer > 0 then
        damageTimer = damageTimer - dt
        if damageTimer <= 0 and HasCollider() then
            local collider = GetCollider(entity)
            if collider and collider.shapes:size() >= 3 then
                collider.shapes[3].isActive = false
            end
        end
    end

    -- Continue dash movement during atk1 animation
    if isDashing then
        local currClip = animator.animator:GetCurrentClip()
        if currClip == "atk1" and not animator.animator:HasFinished() then
            GetRigidBody().velocity = Vec2(dashDirX * ExposedVars.dashSpeed, dashDirY * ExposedVars.dashSpeed)
        else
            GetRigidBody().velocity = Vec2(0, 0)
            isDashing = false
            animator.animator:Play("idle", false)
        end
        return
    end

    -- CHARGING PHASE: count down charge timer, then attack
    if ChargeCD > 0 then
        ChargeCD = ChargeCD - dt

        if ChargeCD <= 0 then
            -- Decide melee vs ranged based on distance at moment of attack
            if distSq <= ExposedVars.meleeRange * ExposedVars.meleeRange then
                PerformMeleeAttack(entity, dx, dy, distSq)
            else
                PerformRangedAttack(entity, dx, dy)
            end
            AttackCD = enemy and enemy.mAttackSpeed or 2.0
        end

    -- COOLDOWN PHASE: wait for attack cooldown
    elseif AttackCD > 0 then
        AttackCD = AttackCD - dt

        if animator.animator:HasFinished() then
            animator.animator:Play("idle", false)
        end

    -- READY: start next charge cycle
    else
        ChargeCD = ExposedVars.chargeTime
        animator.animator:Play("charging_atk", false)
    end
end

function state_exit(entity)
    GetPathFinding().reachedGoal = true

    if HasCollider() then
        local collider = GetCollider(entity)
        if collider and collider.shapes:size() >= 3 then
            collider.shapes[3].isActive = false
        end
    end

    GetRigidBody().velocity = Vec2(0, 0)
    isDashing = false
end

function PerformMeleeAttack(entity, dx, dy, distSq)
    -- Enable damage collider
    if HasCollider() then
        local collider = GetCollider(entity)
        if collider and collider.shapes:size() >= 3 then
            collider.shapes[3].isActive = true
            damageTimer = ExposedVars.damageDuration
        end
    end

    -- Dash towards player
    local length = math.sqrt(distSq)
    if length > 0 then
        dashDirX = dx / length
        dashDirY = dy / length
        isDashing = true
        GetRigidBody().velocity = Vec2(dashDirX * ExposedVars.dashSpeed, dashDirY * ExposedVars.dashSpeed)
    end

    FlipSprite(dx, dy)
    animator.animator:Play("atk1", false)
    PlayEntitySound(EntityID, "wind_enemy_attack", false, 0.3)
end

function PerformRangedAttack(entity, dx, dy)
    local dir = Vec2(dx, dy)
    local angle = math.deg(math.atan(dy, dx))

    local prefab = SpawnPrefab("wind projectile.prefab", Vec2(10000, 10000))
    if prefab == -1 then return end

    local projectile = GetProjectileFrom(prefab)
    if projectile then
        projectile.mStats.damage = enemy.mAttackDamage
    end

    FlipSprite(dx, dy)

    if angle < 0 then angle = angle + 360 end
    AddForce(prefab, Vec2(transform.worldPosition.x, transform.worldPosition.y), dir, projectile.mStats.speed, angle - 180)

    animator.animator:Play("atk2", false)
    PlayEntitySound(EntityID, "wind_enemy_attack", false, 0.3)
end

function FlipSprite(dx, dy)
    if spriteComp then
        local angle = math.deg(math.atan(dy, dx))
        if angle < 0 then angle = angle + 360 end
        if angle >= 90.0 and angle <= 270 then
            spriteComp.flipX = true
        else
            spriteComp.flipX = false
        end
    end
end
