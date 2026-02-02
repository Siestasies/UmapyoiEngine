--copy and paste template for the states
--exposed vars for variable you want that can be editied in editor
ExposedVars = {
    explosionTimer = 2.0,
    damageLinger = 0.5
}

local exploded = false
local explosionTimer = 0
local damageTimer = 0.0

function state_enter(entity)
    explosionTimer = ExposedVars.explosionTimer
    exploded = false
    damageTimer = 0.0
    
    if HasAnimator() then
        local animator = GetAnimator()
        animator.animator:Play("explosion", true)
    end
end

function state_update(entity, dt)
    explosionTimer = explosionTimer - dt
    
    if HasAnimator() then
        local animator = GetAnimator()
        local spriteAnimator = animator.animator
        
        if explosionTimer < 0 and not spriteAnimator:IsPlaying() then
            -- toggle the collider
            local collider = GetCollider(entity)
            if collider and #collider.shapes >= 3 then
                collider.shapes[2].enabled = true
            end
            exploded = true
            damageTimer = ExposedVars.damageLinger
        end
    end
    
    if exploded and damageTimer > 0 then
        damageTimer = damageTimer - dt
    elseif exploded and damageTimer <= 0 then
        DestroyEntityWithChildren(entity)
    end
end
