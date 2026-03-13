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
    enrageSpeedMult = 1.5
}

local animator = nil
local bossTransform = nil

local currentPattern = 1
local patternTimer = 0.0
local bulletTimer = 0.0
local burstTimer = 0.0

local spiralAngle = 0.0
local crossAngle = 0.0

local isEnraged = false
local speedMultiplier = 1.0

function state_enter(entity)
    Log("Boss Phase 3: Final confrontation!")

    currentPattern = 1
    patternTimer = 0.0
    bulletTimer = 0.0
    burstTimer = 0.0
    spiralAngle = 0.0
    crossAngle = 0.0
    isEnraged = false
    speedMultiplier = 1.0

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
        animator.animator:Play("idle2", true)
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

    -- hp bar
    local bar = GetChildren(entity, 1)
    SetActiveEntity(bar, true)
    local bar = GetChildren(entity, 2)
    SetActiveEntity(bar, true)
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

    -- Pattern switching
    patternTimer = patternTimer + dt
    if patternTimer >= ExposedVars.patternSwitchTime then
        patternTimer = 0.0
        currentPattern = currentPattern + 1
        if currentPattern > 4 then
            currentPattern = 1
        end
        Log("Boss Phase 3 pattern: " .. tostring(currentPattern))

        if animator then
            animator.animator:Play("atk", false)
        end
    end

    local adjustedDt = dt * speedMultiplier

    if currentPattern == 1 then
        UpdateSpiral(entity, adjustedDt)
    elseif currentPattern == 2 then
        UpdateRadialBurst(entity, adjustedDt)
    elseif currentPattern == 3 then
        UpdateRotatingCross(entity, adjustedDt)
    elseif currentPattern == 4 then
        -- do nth dps time
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
        audio:play(entity, "Boss Burst")
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
            bTransform.rotation = Vec2(dirX, dirY)
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
