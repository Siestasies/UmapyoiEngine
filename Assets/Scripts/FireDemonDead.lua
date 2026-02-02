ExposedVars = {
    deathDuration = 2.0
}

local dead = false
local animator = nil
local deathTimer = 0.0

function state_enter(entity)
    
    deathTimer = ExposedVars.deathAnimDuration

    -- Play death animation
    if HasAnimator() then
        animator = GetAnimator()
        animator.animator:Play("FireDemonDead", true)
    end
end

function state_update(entity, dt)
    deathTimer = math.max(0, deathTimer - dt)

    if not animator.animator:IsPlaying() then
        dead = true
    end
    
    if dead or deathTimer <= 0 then
        DestroyEntityWithChildren(entity)
    end
end
