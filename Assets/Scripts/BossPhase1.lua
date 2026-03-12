-- BossPhase1: Spawns 3 elite enemies, tracks their deaths
-- Fully self-contained. No shared data with other scripts.
-- Transitions to BossCombine when all elites are dead.

ExposedVars = {
    eliteWaterPrefab = "Elite Water.prefab",
    eliteFirePrefab = "Elite Fire.prefab",
    eliteWindPrefab = "Elite Wind.prefab",
    eliteCount = 3,
    spawnRadius = 120.0,
    spawnDelay = 0.5
}

local spawnTimer = 0.0
local hasSpawned = false
local eliteIds = {}
local elitesAlive = 0

function state_enter(entity)
    Log("Boss Phase 1: Spawning elite guardians")
    spawnTimer = ExposedVars.spawnDelay
    hasSpawned = false
    eliteIds = {}
    elitesAlive = 0

    -- Boss stays hidden
    if HasSprite() then
        local spriteComp = GetSprite()
        spriteComp.alpha = 0
    end

    if HasRigidBody() then
        GetRigidBody().velocity = Vec2(0.0, 0.0)
    end

    if HasPathFinding() then
        GetPathFinding().enabled = false
    end
end

function state_update(entity, dt)
    -- ============ SPAWNING ============
    if not hasSpawned then
        spawnTimer = spawnTimer - dt
        if spawnTimer > 0 then return end

        local bossTransform = GetTransform()
        if not bossTransform then return end

        local bossPos = bossTransform.worldPosition
        local spawnedIds = {}
        local angleStep = (2 * math.pi) / ExposedVars.eliteCount

        for i = 0, ExposedVars.eliteCount - 1 do
            local angle = angleStep * i
            local spawnPos = Vec2(
                bossPos.x + math.cos(angle) * ExposedVars.spawnRadius,
                bossPos.y + math.sin(angle) * ExposedVars.spawnRadius
            )

            local eliteId
            if i == 0 then
                eliteId = SpawnPrefab(ExposedVars.eliteWaterPrefab, spawnPos)
                --SetParent(eliteId, entity)
            elseif i == 1 then
                eliteId = SpawnPrefab(ExposedVars.eliteFirePrefab, spawnPos)
                --SetParent(eliteId, entity)
            else
                eliteId = SpawnPrefab(ExposedVars.eliteWindPrefab, spawnPos)
                --SetParent(eliteId, entity)
            end

            if eliteId ~= -1 then
                table.insert(spawnedIds, eliteId)
                Log("Spawned elite " .. tostring(i + 1) .. " at " ..
                    tostring(spawnPos.x) .. ", " .. tostring(spawnPos.y))
            else
                LogError("Failed to spawn elite prefab")
            end
        end

        eliteIds = spawnedIds
        elitesAlive = #eliteIds
        hasSpawned = true

        Log("Boss Phase 1: " .. tostring(#eliteIds) .. " elites spawned")
        return
    end

    -- ============ TRACK ELITE DEATHS ============
    if KeyPressed(KEY_N) then
        for i, eliteId in ipairs(eliteIds) do
            if IsEntityValid(eliteId) then
                local eliteEnemy = GetEnemyFrom(eliteId)
                eliteEnemy.mHealth = 0;
            end
        end
    end
    
    local alive = 0
    for i, eliteId in ipairs(eliteIds) do
        if IsEntityValid(eliteId) then
            local eliteEnemy = GetEnemyFrom(eliteId)
            if eliteEnemy and eliteEnemy.mHealth > 0 then
                alive = alive + 1
                -- Corpse Tracking
                local eliteTransform = GetTransformFrom(eliteId)
                local childId = GetChildren(entity, i)
                local childTransform = GetTransformFrom(childId)
                childTransform.position.x = eliteTransform.position.x
                childTransform.position.y = eliteTransform.position.y
            end
        end
    end

    if alive ~= elitesAlive then
        elitesAlive = alive
        Log("Elites remaining: " .. tostring(elitesAlive))
    end

    if elitesAlive <= 0 then
        local canChange = true
        for i, eliteId in ipairs(eliteIds) do
            if IsEntityValid(eliteId) then
                canChange = false
            end
        end
        if canChange then
            Log("All elites defeated! Transitioning to combining phase...")
            ChangeState(entity, "BossCombine")
        end
        return
    end

end

function state_exit(entity)
    Log("Boss Phase 1: All elites defeated")
end
