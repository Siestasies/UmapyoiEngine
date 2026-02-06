ExposedVars = {
    attackExitRange = 50.0,
    HoverSpd = 1.5,
    chargeTime = 2.0
}

local AttackCD = 0.0
local ChargeCD = 0.0
local HoverTime = 0.0
local enemy = nil
local baseX = 0.0
local animator = nil
local transform = nil
local isAttacking = false
local spriteComp = nil

function state_enter(entity)
    if HasEnemy() then
        enemy = GetEnemy()
        AttackCD = enemy.mAttackSpeed
    end

    ChargeCD = ExposedVars.chargeTime
    HoverTime = 0.0

    GetRigidBody().velocity = Vec2(0.0, 0.0)
    GetPathFinding().enabled = false

    if HasTransform() then
        transform = GetTransform()
        baseX = transform.position.x
    else
        return
    end

    if HasAnimator() then
        animator = GetAnimator()
    end

    if HasSprite() then
        spriteComp = GetSprite()
    end
end

function state_update(entity, dt)

    local playerId = FindEntityWithComponent("Player")
    if playerId == -1 then return end

    local playerTransform = GetTransformFrom(playerId)
    if not playerTransform then return end

    transform = GetTransform()
    if not transform then return end

    local dir = Vec2(
        playerTransform.worldPosition.x - transform.worldPosition.x,
        playerTransform.worldPosition.y - transform.worldPosition.y
    )


    local angle = math.deg(math.atan(dir.y, dir.x))

    local dx = dir.x
    local dy = dir.y
    local distSq = dx * dx + dy * dy

    if distSq > ExposedVars.attackExitRange * ExposedVars.attackExitRange then
        ChangeState(entity, "WindDemonChase")
        return
    end

    AttackCD = math.max(0, AttackCD - dt)

    if AttackCD > 0 and not isAttacking then

    if animator and animator.animator:HasFinished() then
        animator.animator:Play("idle", false)
    end

    else
        -- start attack if not already started
        if not isAttacking then
            isAttacking = true
            if animator then
                animator.animator:Play("charging_atk2", false)
            end
        end

        -- finish attack when animation ends (even if interrupted once)
        if animator and animator.animator:HasFinished() then

            local prefab = SpawnPrefab("wind projectile.prefab", Vec2(10000, 10000))
            local projectile = GetProjectileFrom(prefab)

            projectile.mStats.damage = enemy.mAttackDamage
            PlayEntitySound(entity, "fire_enemy_attack", false, 0.3)

            if angle < 0 then angle = angle + 360 end

            if spriteComp then
                spriteComp.flipX = (angle >= 90 and angle <= 270)
            end

            AddForce(
                prefab,
                Vec2(transform.worldPosition.x, transform.worldPosition.y),
                dir,
                projectile.mStats.speed,
                angle - 180
            )

            AttackCD = enemy.mAttackSpeed

            if animator then
                animator.animator:Play("atk2", false)
            end

            isAttacking = false
        end
    end

end

function state_exit(entity)
    GetPathFinding().enabled = true
end

function hover(dt)
    if transform then
        HoverTime = HoverTime + dt
        local hoverOffset = math.sin(HoverTime * 2.0) * ExposedVars.HoverSpd
        transform.worldPosition.x = baseX + hoverOffset
    end
end
