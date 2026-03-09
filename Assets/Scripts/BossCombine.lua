-- BossCombine: Transition from phase 1 to phase 2
-- Dead elites play a combining particle animation merging into the boss
-- Once animation finishes, boss reveals itself and transitions to Phase 2

ExposedVars = {
    combineTime = 3.0,         -- total duration of the combine animation
    roomScaleMultiplier = 1.5  -- how much bigger the room gets
}

local combineTimer = 0.0
local hasCombined = false
local animator = nil
local cameraId = -1

function state_enter(entity)
    Log("Boss Combine: Elites merging...")
    combineTimer = ExposedVars.combineTime
    hasCombined = false

    -- Stop boss movement
    if HasRigidBody() then
        GetRigidBody().velocity = Vec2(0.0, 0.0)
    end

    if HasPathFinding() then
        GetPathFinding().enabled = false
    end

    if HasAnimator() then
        animator = GetAnimator()
    end

    cameraId = GetCameraId()

    -- Start particle effect on each dead elite to show them converging
    local eliteIds = GetEliteIds()
    for i, eliteId in ipairs(eliteIds) do
        if IsEntityValid(eliteId) then
            if HasParticleEmitterOn(eliteId) then
                local pe = GetParticleEmitterFrom(eliteId)
                local emitter = pe:GetEmitter(0)
                if emitter then
                    emitter:Play()
                end
            end
            -- Fade out elite sprites
            if HasSpriteOn(eliteId) then
                local spr = GetSpriteFrom(eliteId)
                spr.tintColor = Vec3(0.5, 0.5, 1.0)
            end
        end
    end

    -- Play combine sound
    local audio = GetAudioComponent()
    if audio then
        audio:play(entity, "Boss Combine")
    end
end

function state_update(entity, dt)
    combineTimer = combineTimer - dt

    -- Fade elite sprites towards boss position during combine
    local eliteIds = GetEliteIds()
    local bossTransform = GetTransform()
    local progress = 1.0 - (combineTimer / ExposedVars.combineTime)
    progress = math.max(0.0, math.min(1.0, progress))

    for i, eliteId in ipairs(eliteIds) do
        if IsEntityValid(eliteId) then
            local eliteTransform = GetTransformFrom(eliteId)
            if eliteTransform and bossTransform then
                -- Lerp elite position toward boss center
                local targetX = bossTransform.worldPosition.x
                local targetY = bossTransform.worldPosition.y
                eliteTransform.worldPosition.x = eliteTransform.worldPosition.x +
                    (targetX - eliteTransform.worldPosition.x) * dt * 2.0
                eliteTransform.worldPosition.y = eliteTransform.worldPosition.y +
                    (targetY - eliteTransform.worldPosition.y) * dt * 2.0
            end
        end
    end

    if combineTimer <= 0 and not hasCombined then
        hasCombined = true

        -- Destroy elite entities
        for i, eliteId in ipairs(eliteIds) do
            if IsEntityValid(eliteId) then
                DestroyEntity(eliteId)
            end
        end

        -- Reveal boss
        if HasSprite() then
            local spriteComp = GetSprite()
            spriteComp.visible = true
        end

        -- Enable boss colliders
        if HasCollider() then
            local collider = GetCollider(entity)
            if collider then
                for i = 1, collider.shapes:size() do
                    collider.shapes[i].isActive = true
                end
            end
        end

        -- Play boss reveal animation
        if animator then
            animator.animator:Play("boss_reveal", false)
        end

        -- Expand the room (scale room entity if accessible)
        -- Camera zoom out and lock
        if cameraId ~= -1 and IsEntityValid(cameraId) then
            local camera = GetCameraFrom(cameraId)
            if camera then
                camera.zoom = camera.zoom / ExposedVars.roomScaleMultiplier
                camera.followPlayer = false  -- Lock camera on room
            end
            -- Center camera on boss room
            local camTransform = GetTransformFrom(cameraId)
            local bossPos = GetTransform().worldPosition
            if camTransform then
                camTransform.worldPosition.x = bossPos.x
                camTransform.worldPosition.y = bossPos.y
            end
        end

        local audio = GetAudioComponent()
        if audio then
            audio:play(entity, "Boss Reveal")
        end

        Log("Boss has formed! Transitioning to Phase 2...")
        ChangeState(entity, "BossPhase2")
    end
end

function state_exit(entity)
    Log("Boss Combine: Complete")
end
