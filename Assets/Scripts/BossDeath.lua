-- BossDeath: Boss defeated, plays death animation then destroys self
-- Fully self-contained. No shared data with other scripts.

ExposedVars = {
    deathAnimDuration = 3.0,
    explosionInterval = 0.4,
    explosionPrefab = "BossExplosionVFX.json"
}

local deathTimer = 0.0
local explosionTimer = 0.0
local animator = nil
local hasFinished = false

function state_enter(entity)
    Log("Boss Death: Defeated!")
    deathTimer = ExposedVars.deathAnimDuration
    explosionTimer = 0.0
    hasFinished = false

    if HasRigidBody() then
        GetRigidBody().velocity = Vec2(0.0, 0.0)
    end

    if HasPathFinding() then
        GetPathFinding().enabled = false
    end

    -- Disable all colliders
    if HasCollider() then
        local collider = GetCollider(entity)
        if collider then
            for i = 1, collider.shapes:size() do
                collider.shapes[i].isActive = false
            end
        end
    end

    if HasAnimator() then
        animator = GetAnimator()
        animator.animator:Play("death", false)
    end

    local audio = GetAudioComponent()
    if audio then
        audio:play(entity, "Boss Death")
    end

    -- Restore camera to follow player
    local cameraId = FindEntityWithComponent("Camera")
    if cameraId ~= -1 and IsEntityValid(cameraId) then
        local camera = GetCameraFrom(cameraId)
        if camera then
            camera.followPlayer = true
        end
    end
end

function state_update(entity, dt)
    if hasFinished then return end

    deathTimer = deathTimer - dt

    -- Spawn explosion VFX periodically
    explosionTimer = explosionTimer + dt
    if explosionTimer >= ExposedVars.explosionInterval then
        explosionTimer = 0.0
        SpawnDeathExplosion(entity)
    end

    -- Flash sprite
    if HasSprite() then
        local spriteComp = GetSprite()
        local flash = math.abs(math.sin(deathTimer * 10.0))
        spriteComp.tintColor = Vec3(1.0, flash, flash)
    end

    if deathTimer <= 0 then
        hasFinished = true
        Log("Boss destroyed!")
        DestroyEntity(entity)
    end
end

function SpawnDeathExplosion(entity)
    local transform = GetTransform()
    if not transform then return end

    local bossPos = transform.worldPosition
    local offsetX = (math.random() - 0.5) * 80.0
    local offsetY = (math.random() - 0.5) * 80.0
    local spawnPos = Vec2(bossPos.x + offsetX, bossPos.y + offsetY)

    local vfxId = SpawnPrefab(ExposedVars.explosionPrefab, spawnPos)
    if vfxId ~= -1 then
        PlayOneShotAtPosition(spawnPos.x, spawnPos.y, "explosion", 0.6)
    end
end

function state_exit(entity)
    Log("Boss Death: Exit")
end
