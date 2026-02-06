ExposedVars = {
    StunnedDuration = 3.0
}

local StunCD = 0.0
local animator = nil

function state_enter(entity)
    Log("stunned state entered")
    StunCD = ExposedVars.StunnedDuration

    GetRigidBody().velocity = Vec2(0.0, 0.0)
    GetPathFinding().enabled = false
    
    -- Play stun animation
    if HasAnimator() then
        animator = GetAnimator()
        animator.animator:Play("idle", true)
    end

    if HasParticleEmitter() then
        particleEmitter = GetParticleEmitter()
        local emitter = particleEmitter:GetEmitter(0)
        if emitter then
            emitter:Play()
            Log("Faint particle effect started")
        end
    end
end

function state_update(entity, dt)
     StunCD = math.max(0, StunCD - dt)

    if HasSprite() then
        local spriteComp = GetSprite()
        spriteComp.tintColor = Vec3(0.7, 0.7, 0.7)
    end
    
    if animator and not animator.animator:IsPlaying() and StunCD > 0 then
        ChangeState(entity, "WindDemonIdle")
        return
    end
    
    -- FALLBACK: Timer expired
    if StunCD <= 0 then

        if HasSprite() then
            local spriteComp = GetSprite()
            spriteComp.tintColor = Vec3(1.0, 1.0, 1.0)
        end

        ChangeState(entity, "WindDemonIdle")
    end
end

function state_exit(entity)
    -- Ensure particle effect is stopped when leaving stunned state
    StopParticleEffect()
    GetPathFinding().enabled = true
end

-- Helper function to stop the particle effect
function StopParticleEffect()
    if particleEmitter then
        local emitter = particleEmitter:GetEmitter(0)
        if emitter then
            emitter:Stop()  -- Particles fade out naturally
            Log("Faint particle effect stopped")
        end
    end
end
