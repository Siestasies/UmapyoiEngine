ExposedVars = {
    StunnedDuration = 10.0
}

local StunCD = 0.0

function state_enter(entity)
    Log("stunned state entered")
    StunCD = ExposedVars.StunnedDuration
    
    -- Play stun animation
    if HasAnimator() then
        local animator = GetAnimator()
        animator.animator:Play("WaterDemonStun", true)
    end
end

function state_update(entity, dt)
    StunCD = StunCD - dt
    
    if HasAnimator() then
        local animator = GetAnimator()
        if not animator.animator:IsPlaying() and StunCD > 0 then
            ChangeState(entity, "WaterDemonIdle")
            return
        end
    end
    
    -- FALLBACK: Timer expired
    if StunCD <= 0 then
        ChangeState(entity, "WaterDemonIdle")
    end
end
