ExposedVars = {
    StunnedDuration = 10.0
}

local StunCD = 0.0
local animator = nil

function state_enter(entity)
    Log("stunned state entered")
    StunCD = ExposedVars.StunnedDuration
    
    -- Play stun animation
    if HasAnimator() then
        animator = GetAnimator()
        animator.animator:Play("idle", true)
    end
end

function state_update(entity, dt)
    StunCD = math.max(0, StunCD - dt)
    
    if animator and not animator.animator:IsPlaying() and StunCD > 0 then
        ChangeState(entity, "WindDemonIdle")
        return
    end
    
    -- FALLBACK: Timer expired
    if StunCD <= 0 then
        ChangeState(entity, "WindDemonIdle")
    end
end
