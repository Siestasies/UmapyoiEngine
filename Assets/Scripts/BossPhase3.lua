-- BossPhase3: Boss in center of room, multi-directional bullet hell
-- Fully self-contained. No shared data with other scripts.
-- Boss death is handled by BossBase checking HP when boss is visible.

ExposedVars = {
    bulletSpeed = 50.0,
    bulletDamage = 20,
    bulletPrefab = "boss projectile.prefab",
    patternSwitchTime = 5.0,
    spiralSpeed = 120.0,
    spiralBulletInterval = 0.2,
    burstCount = 12,
    burstInterval = 1.0,
    crossCount = 4,
    crossRotateSpeed = 30.0,
    crossBulletInterval = 0.3,
    enrageHealthPercent = 0.3,
    enrageSpeedMult = 1.5,
    -- Pulsing Rings pattern
    ringBulletCount = 10,
    ringBurstInterval = 0.6,
    -- Expanding Flower pattern
    flowerPetalCount = 5,
    flowerRotateSpeed = 100.0,
    flowerBulletInterval = 0.4,
    flowerSpeedLayers = 1
}

local animator = nil
local bossTransform = nil

local currentPattern = 1
local countPattern = 1
local patternTimer = 0.0
local bulletTimer = 0.0
local burstTimer = 0.0

local spiralAngle = 0.0
local crossAngle = 0.0

-- Pulsing Rings locals
local ringTimer = 0.0
local ringCount = 0

-- Expanding Flower locals
local flowerTimer = 0.0
local flowerAngle = 0.0

local isEnraged = false
local speedMultiplier = 1.0
local phase2timer = 90.0

function state_enter(entity)
    Log("Boss Phase 3: Final confrontation!")

    currentPattern = 5
    patternTimer = 0.0
    bulletTimer = 0.0
    burstTimer = 0.0
    spiralAngle = 0.0
    crossAngle = 0.0
    ringTimer = 0.0
    ringCount = 0
    flowerTimer = 0.0
    flowerAngle = 0.0
    isEnraged = false
    speedMultiplier = 1.0
    countPattern = 1

    bossTransform = GetTransform()

    if HasRigidBody() then
        GetRigidBody().velocity = Vec2(0.0, 0.0)
    end

    if HasPathFinding() then
        GetPathFinding().enabled = false
    end

    -- Lock camera on room center
    local cameraId = FindEntityWithComponent("Camera")
    if cameraId ~= -1 and IsEntityValid(cameraId) then
        local camera = GetCameraFrom(cameraId)
        if camera then
            camera.followPlayer = false
        end
    end

    if HasAnimator() then
        animator = GetAnimator()
    end

    -- Ensure colliders are active for direct damage
    if HasCollider() then
        local collider = GetCollider(entity)
        if collider then
            for i = 1, collider.shapes:size() do
                collider.shapes[i].isActive = true
            end
        end
    end

    local audio = GetAudioComponent()
    if audio then
        audio:play(entity, "Boss Phase3 Music")
    end

    phase2timer = 90.0
end

function state_update(entity, dt)
    bossTransform = GetTransform()
    if not bossTransform then return end

    if HasRigidBody() then
        GetRigidBody().velocity = Vec2(0.0, 0.0)
    end

    if KeyPressed(KEY_N) and HasEnemy() then
        local enemy = GetEnemy()
        enemy.mHealth = 0
    end

    if phase2timer < 0 then
        ChangeState(entity, "BossPhase2")
    else
        phase2timer = phase2timer - dt
    end

    -- Pattern switching
    patternTimer = patternTimer + dt
    if patternTimer >= ExposedVars.patternSwitchTime then
        patternTimer = 0.0
        countPattern = countPattern + 1
        currentPattern = math.random(1,6)
        if countPattern > 4 then
            countPattern = 1
        end
        Log("Boss Phase 3 pattern: " .. tostring(currentPattern))

        if animator then
            animator.animator:Play("attack2", false)
        end
    end

    local adjustedDt = dt * speedMultiplier

    if countPattern < 4 then
        if currentPattern == 1 then
            UpdateSpiral(entity, adjustedDt)
        elseif currentPattern == 2 then
            UpdateRadialBurst(entity, adjustedDt)
        elseif currentPattern == 3 then
            UpdateRotatingCross(entity, adjustedDt)
        elseif currentPattern == 4 then
            UpdatePulsingRings(entity, adjustedDt)
        elseif currentPattern == 5 then
            UpdateExpandingFlower(entity, adjustedDt)
        end
    else
        -- do nth dps time
    end

    if animator and animator.animator:HasFinished() then
        animator.animator:Play("idle2", true)
    end
end

-- ============ BULLET PATTERNS ============

function UpdateSpiral(entity, dt)
    bulletTimer = bulletTimer + dt
    spiralAngle = spiralAngle + (ExposedVars.spiralSpeed * dt)

    if bulletTimer < ExposedVars.spiralBulletInterval then return end
    bulletTimer = 0.0

    local bossPos = bossTransform.worldPosition
    local angleRad = math.rad(spiralAngle)
    local dirX = math.cos(angleRad)
    local dirY = math.sin(angleRad)

    SpawnBullet(entity, bossPos, dirX, dirY, spiralAngle)

    -- Double spiral
    local oppAngleRad = math.rad(spiralAngle + 180)
    SpawnBullet(entity, bossPos, math.cos(oppAngleRad), math.sin(oppAngleRad), spiralAngle + 180)
end

function UpdateRadialBurst(entity, dt)
    burstTimer = burstTimer + dt

    local interval = ExposedVars.burstInterval / speedMultiplier
    if burstTimer < interval then return end
    burstTimer = 0.0

    local bossPos = bossTransform.worldPosition
    local step = 360.0 / ExposedVars.burstCount

    for i = 0, ExposedVars.burstCount - 1 do
        local angleDeg = step * i
        local angleRad = math.rad(angleDeg)
        SpawnBullet(entity, bossPos, math.cos(angleRad), math.sin(angleRad), angleDeg)
    end

    if animator then
        animator.animator:Play("atk", false)
    end

    local audio = GetAudioComponent()
    if audio then
        audio:play(entity, "SFX_Boss_Attack")
    end
end

function UpdateRotatingCross(entity, dt)
    bulletTimer = bulletTimer + dt
    crossAngle = crossAngle + (ExposedVars.crossRotateSpeed * dt)

    if bulletTimer < ExposedVars.crossBulletInterval then return end
    bulletTimer = 0.0

    local bossPos = bossTransform.worldPosition
    local armStep = 360.0 / ExposedVars.crossCount

    for i = 0, ExposedVars.crossCount - 1 do
        local angleDeg = crossAngle + (armStep * i)
        local angleRad = math.rad(angleDeg)
        SpawnBullet(entity, bossPos, math.cos(angleRad), math.sin(angleRad), angleDeg)
    end
end

-- Pulsing Rings: alternating offset 360-degree rings.
-- Each ring is a full circle of bullets; odd rings are rotated by half a step
-- so the gaps in one ring line up with bullets in the next, forcing the player
-- to weave between staggered walls.
function UpdatePulsingRings(entity, dt)
    ringTimer = ringTimer + dt

    local interval = ExposedVars.ringBurstInterval / speedMultiplier
    if ringTimer < interval then return end
    ringTimer = 0.0

    local bossPos = bossTransform.worldPosition
    local count = ExposedVars.ringBulletCount
    local step = 360.0 / count

    -- Offset every other ring by half a step
    local offset = 0.0
    if ringCount % 2 == 1 then
        offset = step * 0.5
    end

    for i = 0, count - 1 do
        local angleDeg = (step * i) + offset
        local angleRad = math.rad(angleDeg)
        SpawnBullet(entity, bossPos, math.cos(angleRad), math.sin(angleRad), angleDeg)
    end

    ringCount = ringCount + 1

    if animator then
        animator.animator:Play("atk", false)
    end

    local audio = GetAudioComponent()
    if audio then
        audio:play(entity, "SFX_Boss_Attack")
    end
end

-- Expanding Flower: rotating petals that each fire bullets at multiple speeds.
-- Each petal arm spawns several bullets simultaneously at different speeds,
-- causing them to spread out into curved petal shapes as the arms rotate.
function UpdateExpandingFlower(entity, dt)
    flowerTimer = flowerTimer + dt
    flowerAngle = flowerAngle + (ExposedVars.flowerRotateSpeed * dt)

    if flowerTimer < ExposedVars.flowerBulletInterval then return end
    flowerTimer = 0.0

    local bossPos = bossTransform.worldPosition
    local petalCount = ExposedVars.flowerPetalCount
    local armStep = 360.0 / petalCount
    local layers = ExposedVars.flowerSpeedLayers

    for i = 0, petalCount - 1 do
        local angleDeg = flowerAngle + (armStep * i)
        local angleRad = math.rad(angleDeg)
        local dirX = math.cos(angleRad)
        local dirY = math.sin(angleRad)

        -- Spawn one bullet per speed layer on each petal arm
        for layer = 1, layers do
            local speedMult = 0.5 + (layer * 0.35)  -- 0.85, 1.2, 1.55 ...
            local spawnPos = Vec2(bossPos.x, bossPos.y)
            local bulletId = SpawnPrefab(ExposedVars.bulletPrefab, spawnPos)

            if bulletId ~= -1 then
                local layerSpeed = ExposedVars.bulletSpeed * speedMult
                if HasRigidBodyOn(bulletId) then
                    local rb = GetRigidBodyFrom(bulletId)
                    rb.velocity = Vec2(dirX * layerSpeed, dirY * layerSpeed)
                end
                if HasProjectileOn(bulletId) then
                    local proj = GetProjectileFrom(bulletId)
                    proj.mStats.damage = ExposedVars.bulletDamage
                    proj.mStats.speed = layerSpeed
                end
                if HasTransformOn(bulletId) then
                    local bTransform = GetTransformFrom(bulletId)
                    bTransform.rotation = Vec2(dirX, dirY) * 10
                end
            end
        end
    end
end

-- ============ HELPERS ============

function SpawnBullet(entity, bossPos, dirX, dirY, angleDeg)
    local spawnPos = Vec2(bossPos.x, bossPos.y)
    local bulletId = SpawnPrefab(ExposedVars.bulletPrefab, spawnPos)

    if bulletId ~= -1 then
        if HasRigidBodyOn(bulletId) then
            local rb = GetRigidBodyFrom(bulletId)
            rb.velocity = Vec2(dirX * ExposedVars.bulletSpeed, dirY * ExposedVars.bulletSpeed)
        end
        if HasProjectileOn(bulletId) then
            local proj = GetProjectileFrom(bulletId)
            proj.mStats.damage = ExposedVars.bulletDamage
            proj.mStats.speed = ExposedVars.bulletSpeed
        end
        if HasTransformOn(bulletId) then
            local bTransform = GetTransformFrom(bulletId)
            bTransform.rotation = Vec2(dirX, dirY) * 10
        end
    end
end

function state_exit(entity)
    Log("Boss Phase 3: Exiting")

    if HasSprite() then
        local spriteComp = GetSprite()
        spriteComp.tintColor = Vec3(1.0, 1.0, 1.0)
    end
end