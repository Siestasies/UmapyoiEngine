ExposedVars = {
    deathDuration = 2.0
}

local dead = false
local animator = nil
local deathTimer = 0.0
local audio

function state_enter(entity)
    
    deathTimer = ExposedVars.deathDuration

    GetRigidBody().velocity = Vec2(0.0, 0.0)
    GetPathFinding().enabled = false

    -- Play death animation
    if HasAnimator() then
        animator = GetAnimator()
        animator.animator:Play("dead", true)
    end

    audio = GetAudioComponent()
    audio:play(EntityID, "WindDemonDeath")
end

function state_update(entity, dt)
    deathTimer = math.max(0, deathTimer - dt)

    if not animator.animator:IsPlaying() then
        dead = true
    end
    
    if dead or deathTimer <= 0 then
        DestroyWithChildren(entity)
    end
end


