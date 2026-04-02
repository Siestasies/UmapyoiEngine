-- BossPhase2: Boss at top of room, bullet hell + totems
-- Fully self-contained. No shared data with other scripts.
-- Spawns totems, tracks their deaths, transitions to Phase 3 when all destroyed.

ExposedVars = {
    topOffsetY = 50.0,
    --bulletInterval = 0.2,
    bulletInterval = 0.3,
    bulletSpeed = 100.0,
    bulletDamage = 15,
    bulletPrefab = "boss projectile.prefab",
    waterTotemPrefab = "Water Totem.prefab",
    fireTotemPrefab = "Fire Totem.prefab",
    windTotemPrefab = "Wind Totem.prefab",
    totemCount = 3,
    totemSpawnDelay = 1.5,
    spreadAngle = 150.0,
    bulletsPerVolley = 8,
    volleyInterval = 0.8,
    sweepSpeed = 40.0,
    patternSwitchTime = 6.0,
    finalTransformationTime = 3.5
}

local bulletTimer = 0.0
local volleyTimer = 0.0
local totemSpawnTimer = 0.0
local totemsSpawned = false
local animator = nil
local bossTransform = nil
local currentPattern = 1
local patternTimer = 0.0
local sweepAngle = 0.0
local sweepDir = 1
local targetY = 0.0
local isMoving = false

-- Totem tracking
local totemIds = {}
local totemsAlive = 0
local finalPhase = false
local finalTransformationItr = 1

local totemPos1 = Vec2(-192, -170)
local totemPos2 = Vec2(-96 , -195.5)
local totemPos3 = Vec2(0 , -170)

local wasNotFinalPhase = true;

function state_enter(entity)
    Log("Boss Phase 2: Bullet hell + Totems")

    bulletTimer = 0.0
    volleyTimer = 0.0
    totemSpawnTimer = ExposedVars.totemSpawnDelay
    totemsSpawned = false
    finalPhase = false
    wasNotFinalPhase = true
    currentPattern = 1
    patternTimer = 0.0
    sweepAngle = -ExposedVars.spreadAngle / 2
    totemIds = {}
    totemsAlive = 0

    -- Position boss at top of room
    bossTransform = GetTransform()
    if bossTransform then
        targetY = bossTransform.position.y + ExposedVars.topOffsetY
        isMoving = true
    end

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
        audio:play(entity, "SFX_Boss_Spawn2")
    end
end

function state_update(entity, dt)
    bossTransform = GetTransform()
    if not bossTransform then return end

    if HasRigidBody() then
        GetRigidBody().velocity = Vec2(0.0, 0.0)
    end

    if isMoving then
        local dy = targetY - bossTransform.position.y
        if math.abs(dy) < 1.0 then
            bossTransform.position.y = targetY
            isMoving = false
        else
            bossTransform.position.y = bossTransform.position.y + dy * dt * 3.0
        end
        return
    end

    if finalPhase then 
        ExposedVars.finalTransformationTime = ExposedVars.finalTransformationTime - dt
        if animator and animator.animator:HasFinished() then
            finalTransformationItr = finalTransformationItr + 1;
            if finalTransformationItr == 2 then
                animator.animator:Play("final_phase_transition2", false)
            elseif finalTransformationItr == 3 then
                animator.animator:Play("final_phase_transition3", false)
            end
        end
        if ExposedVars.finalTransformationTime <= 0 and not isMoving then
            ChangeState(entity, "BossPhase3")
            
        end
    end

    -- Spawn totems after delay
    if not totemsSpawned then
        totemSpawnTimer = totemSpawnTimer - dt
        if totemSpawnTimer <= 0 then
            SpawnTotems(entity)
            totemsSpawned = true
        end
    end

    -- ============ TRACK TOTEM DEATHS ============
    if KeyPressed(KEY_N) then
        for i, totemId in ipairs(totemIds) do
            if IsEntityValid(totemId) then
                local totemEnemy = GetEnemyFrom(totemId)
                totemEnemy.mHealth = 0;
            end
        end
    end

    if totemsSpawned and totemsAlive > 0 then
        local alive = 0
        for i, totemId in ipairs(totemIds) do
            if IsEntityValid(totemId) then
                local totemEnemy = GetEnemyFrom(totemId)
                if totemEnemy and totemEnemy.mHealth > 0 then
                    alive = alive + 1
                end
            end
        end

        if alive ~= totemsAlive then
            totemsAlive = alive
            Log("Totems remaining: " .. tostring(totemsAlive))
        end

        if totemsAlive <= 0 then
            Log("All totems destroyed! Transitioning to phase 3...")
            if animator then
                animator.animator:Play("final_phase_transition1", false)
            end
            if wasNotFinalPhase == true then
                GetAudioComponent():play(entity, "SFX_Boss_Spawn2")
                wasNotFinalPhase = false
            end
            finalPhase = true
            isMoving = true
            targetY = bossTransform.position.y - ExposedVars.topOffsetY
            return
        end
        
        -- ============ BULLET PATTERNS ============
        
        patternTimer = patternTimer + dt
        if patternTimer >= ExposedVars.patternSwitchTime then
            patternTimer = 0.0
            currentPattern = currentPattern + 1
            if currentPattern > 3 then
                currentPattern = 1
            end
            if animator then
                animator.animator:Play("attack1", false)
            end
            Log("Boss switching to pattern: " .. tostring(currentPattern))
        end
        
        if currentPattern == 1 then
            UpdateFanPattern(entity, dt)
        elseif currentPattern == 2 then
            UpdateSweepPattern(entity, dt)
        elseif currentPattern == 3 then
            UpdateRainPattern(entity, dt)
        end
    end

    if not finalPhase and animator and animator.animator:HasFinished() then
        animator.animator:Play("idle", true)
    end
end

-- ============ BULLET PATTERNS ============
function UpdateFanPattern(entity, dt)
    volleyTimer = volleyTimer + dt

    if volleyTimer < ExposedVars.volleyInterval then return end
    volleyTimer = 0.0

    local bossPos = bossTransform.worldPosition
    local count = ExposedVars.bulletsPerVolley
    local totalSpread = ExposedVars.spreadAngle
    local step = totalSpread / (count - 1)
    local startAngle = -totalSpread / 2

    -- Sway the entire fan left/right over time
    local swayAmount = 12.0  -- degrees of sway in each direction
    local swaySpeed = 2.5    -- how fast it oscillates
    local sway = math.sin(patternTimer * swaySpeed) * swayAmount

    for i = 0, count - 1 do
        -- -90 points downward, then spread across the fan + sway offset
        local angleDeg = -90 + startAngle + (step * i) + sway
        local angleRad = math.rad(angleDeg)
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
                proj.mStats.damage = ExposedVars.bulletDamage
                proj.mStats.speed = ExposedVars.bulletSpeed
            end
            if HasTransformOn(bulletId) then
                local bTransform = GetTransformFrom(bulletId)
                bTransform.rotation = Vec2(dirX, dirY)
            end
        end
    end

    if animator then
        animator.animator:Play("atk", false)
    end

    local audio = GetAudioComponent()
    if audio then
        audio:play(entity, "SFX_Boss_Attack")
    end
end

function UpdateSweepPattern(entity, dt)
    bulletTimer = bulletTimer + dt
    if bulletTimer < ExposedVars.bulletInterval then return end
    bulletTimer = 0.0

    sweepAngle = sweepAngle + (ExposedVars.sweepSpeed * dt * sweepDir * 10)
    if sweepAngle > ExposedVars.spreadAngle / 2 then
        sweepDir = -1
    elseif sweepAngle < -ExposedVars.spreadAngle / 2 then
        sweepDir = 1
    end

    local bossPos = bossTransform.worldPosition
    local angleRad = math.rad(sweepAngle - 90)
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
            proj.mStats.damage = ExposedVars.bulletDamage
            proj.mStats.speed = ExposedVars.bulletSpeed
        end
        if HasTransformOn(bulletId) then
            local bTransform = GetTransformFrom(bulletId)
            local rotRad = math.rad(sweepAngle - 90)
            bTransform.rotation = Vec2(math.cos(rotRad), math.sin(rotRad))
        end
    end
end

function UpdateRainPattern(entity, dt)
    bulletTimer = bulletTimer + dt
    if bulletTimer < ExposedVars.bulletInterval * 0.5 then return end
    bulletTimer = 0.0

    local bossPos = bossTransform.worldPosition
    local offsetX = (math.random() - 0.5) * 400.0
    local spawnPos = Vec2(bossPos.x + offsetX, bossPos.y)

    local bulletId = SpawnPrefab(ExposedVars.bulletPrefab, spawnPos)

    if bulletId ~= -1 then
        local driftX = (math.random() - 0.5) * 60.0
        if HasRigidBodyOn(bulletId) then
            local rb = GetRigidBodyFrom(bulletId)
            rb.velocity = Vec2(driftX, -ExposedVars.bulletSpeed)
        end
        if HasProjectileOn(bulletId) then
            local proj = GetProjectileFrom(bulletId)
            proj.mStats.damage = ExposedVars.bulletDamage
            proj.mStats.speed = ExposedVars.bulletSpeed
        end
        if HasTransformOn(bulletId) then
            local bTransform = GetTransformFrom(bulletId)
            bTransform.rotation = Vec2(0, -1)
        end
    end
end

-- ============ TOTEM SPAWNING ============

function SpawnTotems(entity)
    totemIds = {}
    local bossPos = bossTransform.worldPosition
    local totemCount = ExposedVars.totemCount

    for i = 1, totemCount do
        local spawnPos
        local totemId
            if i == 1 then
                spawnPos = totemPos1
                totemId = SpawnPrefab(ExposedVars.waterTotemPrefab, totemPos1)
                --SetParent(eliteId, entity)
            elseif i == 2 then
                spawnPos = totemPos2
                totemId = SpawnPrefab(ExposedVars.windTotemPrefab, totemPos2)
                --SetParent(eliteId, entity)
            else
                spawnPos = totemPos3
                totemId = SpawnPrefab(ExposedVars.fireTotemPrefab, totemPos3)
                --SetParent(eliteId, entity)
            end
        if totemId ~= -1 then
            table.insert(totemIds, totemId)
            Log("Spawned totem " .. tostring(i) .. " at " .. tostring(spawnPos.x) .. ", " .. tostring(spawnPos.y))
        else
            LogError("Failed to spawn totem prefab")
        end
    end

    totemsAlive = #totemIds

    local audio = GetAudioComponent()
    if audio then
        audio:play(entity, "Totem Spawn")
    end

    Log("Spawned " .. tostring(#totemIds) .. " totems")
end

function state_exit(entity)
    Log("Boss Phase 2: Complete")
    for i, totemId in ipairs(totemIds) do
        if IsEntityValid(totemId) then
            DestroyWithChildren(totemId)
        end
    end
end
