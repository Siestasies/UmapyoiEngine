-- BossCombine: Transition from phase 1 to phase 2
-- Fully self-contained. No shared data with other scripts.
-- Finds remaining elite corpses via FindEntitiesWithComponent("Enemy"),
-- plays a combining animation, then reveals the boss and transitions to Phase 2.

ExposedVars = {
    combineTime = 3.0,
    revealTime = 2.0,
    roomScaleMultiplier = 1.5
}

local combineTimer = 0.0
local hasCombined = false
local animator = nil
local cameraId = -1
local eliteCorpses = {}
local vfx

function state_enter(entity)
    Log("Boss Combine: Elites merging...")
    combineTimer = ExposedVars.combineTime
    hasCombined = false

    if HasRigidBody() then
        GetRigidBody().velocity = Vec2(0.0, 0.0)
    end

    if HasPathFinding() then
        GetPathFinding().enabled = false
    end

    if HasAnimator() then
        animator = GetAnimator()
    end

    cameraId = FindEntityWithComponent("Camera")

    -- Find elite corpses: children of the boss entity
    eliteCorpses = {}
    for i = 1, 3 do
        local childId = GetChildren(entity, i)
        if IsEntityValid(childId) then
            SetActiveEntity(childId, true)
            table.insert(eliteCorpses, childId)
        end
    end

    for i, corpseId in ipairs(eliteCorpses) do
        if IsEntityValid(corpseId) then
            RemoveParentSpecial(corpseId)
        end
    end

    Log("Found " .. tostring(#eliteCorpses) .. " elite corpses to combine")

    -- Start particle effects on corpses
    for i, corpseId in ipairs(eliteCorpses) do
        if IsEntityValid(corpseId) then
            if HasParticleEmitterOn(corpseId) then
                local pe = GetParticleEmitterFrom(corpseId)
                local emitter = pe:GetEmitter(0)
                if emitter then
                    emitter:Play()
                end
            end
            if HasSpriteOn(corpseId) then
                local spr = GetSpriteFrom(corpseId)
                spr.tintColor = Vec3(0.5, 0.5, 1.0)
            end
        end
    end

    local audio = GetAudioComponent()
    if audio then
        audio:play(entity, "Boss Combine")
    end

    vfx = GetChildren(entity, 0)
end

function state_update(entity, dt)
    combineTimer = combineTimer - dt

    -- Lerp corpses toward boss center
    local bossTransform = GetTransform()
    if bossTransform then
        for i, corpseId in ipairs(eliteCorpses) do
            if IsEntityValid(corpseId) then
                local corpseTransform = GetTransformFrom(corpseId)
                if corpseTransform then
                    local targetX = bossTransform.position.x
                    local targetY = bossTransform.position.y
                    corpseTransform.position.x = corpseTransform.position.x +
                        (targetX - corpseTransform.position.x) * dt * 2.0
                    corpseTransform.position.y = corpseTransform.position.y +
                        (targetY - corpseTransform.position.y) * dt * 2.0
                end
            end
        end
    end

    if combineTimer <= 0 and not hasCombined then
        hasCombined = true

        -- Destroy corpses
        for i, corpseId in ipairs(eliteCorpses) do
            if IsEntityValid(corpseId) then
                DestroyWithChildren(corpseId)
            end
        end

        -- Reveal boss
        if HasSprite() then
            local spriteComp = GetSprite()
            spriteComp.alpha = 1
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

        if animator then
            animator.animator:Play("boss_reveal", false)
        end
        local vfxAnimator = GetAnimatorFrom( vfx )
        if vfxAnimator then 
            vfxAnimator.animator:Play("boss_reveal", false)
        end
        if vfx then 
            SetActiveEntity(vfx, true)
        end


        -- Camera zoom out and lock
        if cameraId ~= -1 and IsEntityValid(cameraId) then
            local camera = GetCameraFrom(cameraId)
            if camera then
                camera.zoom = camera.zoom / ExposedVars.roomScaleMultiplier
                camera.followPlayer = false
            end
            local camTransform = GetTransformFrom(cameraId)
            local bossPos = GetTransform().position
            if camTransform then
                camTransform.position.x = bossPos.x
                camTransform.position.y = bossPos.y
            end
        end

        local audio = GetAudioComponent()
        if audio then
            audio:play(entity, "Boss Reveal")
        end

        Log("Boss has formed! Transitioning to Phase 2...")
    end
    
    if hasCombined then
        if revealTime <= 0 then 
            ChangeState(entity, "BossPhase2")
        else
            if animator and animator.animator:HasFinished() then
                animator.animator:Play("boss_reveal_to_idle", false)
            end
            revealTime = revealTime - dt;
        end
    end

end

function state_exit(entity)
    if vfx then 
            SetActiveEntity(vfx, false)
    end
    Log("Boss Combine: Complete")
end
