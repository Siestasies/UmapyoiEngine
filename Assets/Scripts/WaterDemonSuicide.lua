ExposedVars = {
    explosionTimer = 2.0,
    damageLinger = 0.5
}

local exploded = false
local explosionTimer = 0
local damageTimer = 0.0
local isCharging = false
local shieldBroken = false  -- Track shield break phase
local vfxAnimator = nil;

function state_enter(entity)
    explosionTimer = ExposedVars.explosionTimer
    exploded = false
    damageTimer = 0.0
    isCharging = false
    shieldBroken = false  -- Start with shield breaking

    if HasPathFinding() then 
        local pf = GetPathFinding()
        pf.goal.x = GetTransform().worldPosition.x
        pf.goal.y = GetTransform().worldPosition.y

        pf.reachedGoal = true
    end 
    
    if HasAnimator() then
        local animator = GetAnimator()
        animator.animator:Play("shield_broken", false)
    end

    if HasChildren(EntityID, 0) then

        local vfxID = GetChildren(EntityID, 0)

        if HasAnimatorOn(vfxID) then
            vfxAnimator = GetAnimatorFrom(vfxID)
        end

    end

end

function state_update(entity, dt)
    if HasAnimator() then
        local animator = GetAnimator()
        
        if not shieldBroken then
            -- Wait for shield_broken animation to finish
            if animator.animator:HasFinished() and 
               animator.animator:GetCurrentClip() == "shield_broken" then
                shieldBroken = true
                isCharging = true  -- Now start charging phase
            end
            
        elseif isCharging then
            -- PHASE 1: Charging explosion
            explosionTimer = explosionTimer - dt
            
            -- Play charging animation (only once)
            if animator.animator:GetCurrentClip() ~= "charging_explode" then
                animator.animator:Play("charging_explode", false)
            end
            
            -- Check if charge timer finished
            if explosionTimer <= 0 then
                isCharging = false
                
                -- Enable damage collider
                local collider = GetCollider(entity)
                if collider and collider.shapes:size() >= 3 then
                    collider.shapes[3].isActive = true
                end
                
                -- Play explosion animation
                animator.animator:Play("explode", false)

                if vfxAnimator ~= nil then
                    vfxAnimator.animator:Play("splash", true)
                end
                
                exploded = true
                damageTimer = ExposedVars.damageLinger
            end
            
        elseif exploded then
            -- PHASE 2: Explosion active, count down damage linger
            damageTimer = damageTimer - dt
            
            if damageTimer <= 0 then
                Log("sayonara")
                DestroyWithChildren(entity)
            end
        end
    end
end