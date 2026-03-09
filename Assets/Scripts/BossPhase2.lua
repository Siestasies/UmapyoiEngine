-- BossPhase2: Boss at top of room, bullet hell + totems
-- Boss positions itself at the top of the room and shoots projectiles downward
-- Totems spawn that the player must destroy using advantageous elements
-- Base script tracks totem deaths and transitions to Phase 3 when all are gone

ExposedVars = {
    topOffsetY = 200.0,            -- how far above room center the boss sits
    bulletInterval = 0.3,          -- time between bullet spawns
    bulletSpeed = 300.0,
    bulletDamage = 15,
    bulletPrefab = "BossBullet.json",
    totemPrefab = "BossTotem.json",
    totemCount = 3,
    totemSpawnDelay = 1.5,         -- delay before totems appear after phase starts
    spreadAngle = 60.0,            -- spread angle for bullet fan (degrees)
    bulletsPerVolley = 5,          -- number of bullets per volley
    volleyInterval = 1.5,          -- time between volleys
    sweepSpeed = 40.0,             -- horizontal sweep speed for aimed patterns
    patternSwitchTime = 6.0        -- time before switching bullet pattern
}

local bulletTimer = 0.0
local volleyTimer = 0.0
local totemSpawnTimer = 0.0
local totemsSpawned = false
local animator = nil
local bossTransform = nil
local currentPattern = 1           -- 1 = fan, 2 = sweep, 3 = rain
local patternTimer = 0.0
local sweepAngle = 0.0
local sweepDir = 1

function state_enter(entity)
    Log("Boss Phase 2: Bullet hell + Totems")

    bulletTimer = 0.0
    volleyTimer = 0.0
    totemSpawnTimer = ExposedVars.totemSpawnDelay
    totemsSpawned = false
    currentPattern = 1
    patternTimer = 0.0
    sweepAngle = -ExposedVars.spreadAngle / 2

    -- Position boss at top of room
    bossTransform = GetTransform()
    if bossTransform then
        bossTransform.worldPosition.y = bossTransform.worldPosition.y + ExposedVars.topOffsetY
    end

    -- Stop movement
    if HasRigidBody() then
        GetRigidBody().velocity = Vec2(0.0, 0.0)
    end

    if HasPathFinding() then
        GetPathFinding().enabled = false
    end

    if HasAnimator() then
        animator = GetAnimator()
        animator.animator:Play("idle", true)
    end

    local audio = GetAudioComponent()
    if audio then
        audio:play(entity, "Boss Phase2 Music")
    end
end

function state_update(entity, dt)
    bossTransform = GetTransform()
    if not bossTransform then return end

    -- Spawn totems after delay
    if not totemsSpawned then
        totemSpawnTimer = totemSpawnTimer - dt
        if totemSpawnTimer <= 0 then
            SpawnTotems(entity)
            totemsSpawned = true
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
        Log("Boss switching to pattern: " .. tostring(currentPattern))
    end

    -- Fire bullets based on current pattern
    if currentPattern == 1 then
        UpdateFanPattern(entity, dt)
    elseif currentPattern == 2 then
        UpdateSweepPattern(entity, dt)
    elseif currentPattern == 3 then
        UpdateRainPattern(entity, dt)
    end
end

-- ============ BULLET PATTERNS ============

-- Pattern 1: Fan of bullets aimed downward
function UpdateFanPattern(entity, dt)
    volleyTimer = volleyTimer + dt
    if volleyTimer < ExposedVars.volleyInterval then return end
    volleyTimer = 0.0

    if animator then
        animator.animator:Play("atk", false)
    end

    local bossPos = bossTransform.worldPosition
    local halfSpread = ExposedVars.spreadAngle / 2
    local step = ExposedVars.spreadAngle / math.max(1, ExposedVars.bulletsPerVolley - 1)

    for i = 0, ExposedVars.bulletsPerVolley - 1 do
        local angleDeg = -halfSpread + (step * i)
        local angleRad = math.rad(angleDeg - 90)  -- -90 to point downward
        local dirX = math.cos(angleRad)
        local dirY = math.sin(angleRad)

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
            -- Rotate bullet to face direction
            if HasTransformOn(bulletId) then
                local bTransform = GetTransformFrom(bulletId)
                bTransform.rotation = angleDeg - 90
            end
        end
    end

    local audio = GetAudioComponent()
    if audio then
        audio:play(entity, "Boss Shoot")
    end
end

-- Pattern 2: Sweeping beam of bullets left to right
function UpdateSweepPattern(entity, dt)
    bulletTimer = bulletTimer + dt
    if bulletTimer < ExposedVars.bulletInterval then return end
    bulletTimer = 0.0

    -- Sweep the angle back and forth
    sweepAngle = sweepAngle + (ExposedVars.sweepSpeed * dt * sweepDir * 10)
    if sweepAngle > ExposedVars.spreadAngle / 2 then
        sweepDir = -1
    elseif sweepAngle < -ExposedVars.spreadAngle / 2 then
        sweepDir = 1
    end

    local bossPos = bossTransform.worldPosition
    local angleRad = math.rad(sweepAngle - 90)  -- -90 to point downward
    local dirX = math.cos(angleRad)
    local dirY = math.sin(angleRad)

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
            bTransform.rotation = sweepAngle - 90
        end
    end
end

-- Pattern 3: Random rain of bullets from boss width
function UpdateRainPattern(entity, dt)
    bulletTimer = bulletTimer + dt
    if bulletTimer < ExposedVars.bulletInterval * 0.5 then return end  -- faster rain
    bulletTimer = 0.0

    local bossPos = bossTransform.worldPosition
    -- Spawn across a wide horizontal range
    local offsetX = (math.random() - 0.5) * 400.0
    local spawnPos = Vec2(bossPos.x + offsetX, bossPos.y)

    local bulletId = SpawnPrefab(ExposedVars.bulletPrefab, spawnPos)

    if bulletId ~= -1 then
        -- Slight random horizontal drift
        local driftX = (math.random() - 0.5) * 60.0
        if HasRigidBodyOn(bulletId) then
            local rb = GetRigidBodyFrom(bulletId)
            rb.velocity = Vec2(driftX, -ExposedVars.bulletSpeed)
        end
        if HasProjectileOn(bulletId) then
            local proj = GetProjectileFrom(bulletId)
            proj.mDamage = ExposedVars.bulletDamage
            proj.mSpeed = ExposedVars.bulletSpeed
        end
        if HasTransformOn(bulletId) then
            local bTransform = GetTransformFrom(bulletId)
            bTransform.rotation = -90
        end
    end
end

-- ============ TOTEM SPAWNING ============

function SpawnTotems(entity)
    local totemIds = {}
    local bossPos = bossTransform.worldPosition
    local totemCount = ExposedVars.totemCount

    for i = 1, totemCount do
        -- Spread totems evenly across the room floor
        local fraction = (i - 1) / math.max(1, totemCount - 1)
        local offsetX = (fraction - 0.5) * 300.0  -- spread across 300 units
        local spawnPos = Vec2(
            bossPos.x + offsetX,
            bossPos.y - ExposedVars.topOffsetY * 0.8  -- place near room floor
        )

        local totemId = SpawnPrefab(ExposedVars.totemPrefab, spawnPos)
        if totemId ~= -1 then
            table.insert(totemIds, totemId)
            Log("Spawned totem " .. tostring(i) .. " at " .. tostring(spawnPos.x) .. ", " .. tostring(spawnPos.y))
        else
            LogError("Failed to spawn totem prefab")
        end
    end

    -- Register totems with base script for tracking
    RegisterTotems(totemIds)

    local audio = GetAudioComponent()
    if audio then
        audio:play(entity, "Totem Spawn")
    end

    Log("Spawned " .. tostring(#totemIds) .. " totems")
end

function state_exit(entity)
    Log("Boss Phase 2: Complete")

    -- Clean up remaining totems if any
    -- (base script handles tracking, but just in case)
end
