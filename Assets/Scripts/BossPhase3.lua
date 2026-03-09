-- BossPhase3: Boss in center of room, multi-directional bullet hell
-- Boss moves to the center and fires projectiles in various patterns
-- Player must dodge and deal damage directly to the boss
-- Base script tracks boss health and transitions to BossDeath at 0 HP

ExposedVars = {
    bulletSpeed = 350.0,
    bulletDamage = 20,
    bulletPrefab = "BossBullet.json",
    patternSwitchTime = 5.0,     -- time between pattern changes
    spiralSpeed = 120.0,          -- degrees per second for spiral
    spiralBulletInterval = 0.08,  -- time between spiral bullets
    burstCount = 12,              -- bullets per radial burst
    burstInterval = 2.0,          -- time between bursts
    crossCount = 4,               -- number of cross arms
    crossRotateSpeed = 30.0,      -- degrees per second rotation
    crossBulletInterval = 0.12,   -- time between cross bullets
    enrageHealthPercent = 0.3,    -- below this % boss attacks faster
    enrageSpeedMult = 1.5         -- multiplier when enraged
}

local animator = nil
local bossTransform = nil

-- Pattern state
local currentPattern = 1           -- 1 = spiral, 2 = radial burst, 3 = rotating cross
local patternTimer = 0.0
local bulletTimer = 0.0
local burstTimer = 0.0

-- Spiral tracking
local spiralAngle = 0.0

-- Cross tracking
local crossAngle = 0.0

-- Enrage
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

    -- Stop pathfinding
    if HasRigidBody() then
        GetRigidBody().velocity = Vec2(0.0, 0.0)
    end

    if HasPathFinding() then
        GetPathFinding().enabled = false
    end

    -- Re-enable camera follow or keep locked on center
    local cameraId = GetCameraId()
    if cameraId ~= -1 and IsEntityValid(cameraId) then
        local camera = GetCameraFrom(cameraId)
        if camera then
            camera.followPlayer = false  -- keep locked on room center
        end
    end

    if HasAnimator() then
        animator = GetAnimator()
        animator.animator:Play("phase3_intro", false)
    end

    -- Enable all colliders for direct damage
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
end

function state_update(entity, dt)
    bossTransform = GetTransform()
    if not bossTransform then return end

    -- Enrage check
    if not isEnraged and HasEnemy() then
        local enemy = GetEnemy()
        if enemy and enemy.mMaxHealth > 0 then
            local healthPercent = enemy.mHealth / enemy.mMaxHealth
            if healthPercent <= ExposedVars.enrageHealthPercent then
                isEnraged = true
                speedMultiplier = ExposedVars.enrageSpeedMult
                Log("Boss ENRAGED!")

                if HasSprite() then
                    local spriteComp = GetSprite()
                    spriteComp.tintColor = Vec3(1.0, 0.3, 0.3)
                end

                local audio = GetAudioComponent()
                if audio then
                    audio:play(entity, "Boss Enrage")
                end
            end
        end
    end

    -- Play idle anim when intro finishes
    if animator and animator.animator:HasFinished() then
        if animator.animator:GetCurrentClip() == "phase3_intro" then
            animator.animator:Play("idle", true)
        end
    end

    -- Pattern switching
    patternTimer = patternTimer + dt
    if patternTimer >= ExposedVars.patternSwitchTime then
        patternTimer = 0.0
        currentPattern = currentPattern + 1
        if currentPattern > 3 then
            currentPattern = 1
        end
        Log("Boss Phase 3 pattern: " .. tostring(currentPattern))

        if animator then
            animator.animator:Play("atk", false)
        end
    end

    -- Fire based on current pattern
    local adjustedDt = dt * speedMultiplier

    if currentPattern == 1 then
        UpdateSpiral(entity, adjustedDt)
    elseif currentPattern == 2 then
        UpdateRadialBurst(entity, adjustedDt)
    elseif currentPattern == 3 then
        UpdateRotatingCross(entity, adjustedDt)
    end
end

-- ============ BULLET PATTERNS ============

-- Pattern 1: Continuous spiral of bullets
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

    -- Double spiral (opposite direction)
    local oppAngleRad = math.rad(spiralAngle + 180)
    local oppDirX = math.cos(oppAngleRad)
    local oppDirY = math.sin(oppAngleRad)
    SpawnBullet(entity, bossPos, oppDirX, oppDirY, spiralAngle + 180)
end

-- Pattern 2: Periodic radial burst (all directions at once)
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
        local dirX = math.cos(angleRad)
        local dirY = math.sin(angleRad)

        SpawnBullet(entity, bossPos, dirX, dirY, angleDeg)
    end

    if animator then
        animator.animator:Play("atk", false)
    end

    local audio = GetAudioComponent()
    if audio then
        audio:play(entity, "Boss Burst")
    end
end

-- Pattern 3: Rotating cross arms that spray bullets
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
        local dirX = math.cos(angleRad)
        local dirY = math.sin(angleRad)

        SpawnBullet(entity, bossPos, dirX, dirY, angleDeg)
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
            proj.mDamage = ExposedVars.bulletDamage
            proj.mSpeed = ExposedVars.bulletSpeed
        end
        if HasTransformOn(bulletId) then
            local bTransform = GetTransformFrom(bulletId)
            bTransform.rotation = angleDeg
        end
    end
end

function state_exit(entity)
    Log("Boss Phase 3: Exiting")

    -- Reset tint if enraged
    if HasSprite() then
        local spriteComp = GetSprite()
        spriteComp.tintColor = Vec3(1.0, 1.0, 1.0)
    end
end
