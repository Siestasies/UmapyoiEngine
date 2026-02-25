ExposedVars = {
    --empty for now
    lifeTime = 5.0
}

local animator = nil
local isAlive = true
local parentID = nil
local shape = nil

function Start()
end

function Update(dt)

    lifeTime = lifeTime - dt

    if animator == nil then
        if HasAnimator() then
            animator = GetAnimator()
        end

        parentID = GetParent(EntityID)

        local collider = GetColliderFrom(parentID)

        shape = collider.shapes[1]
    end

    if animator.animator:GetCurrentClip() == "shield up" and 
    animator.animator:HasFinished() then 
        local transform = GetTransform()
        transform.position = Vec2(0,0 + shape.offset.y)
        animator.animator:Play("shielding", false)
    end

    if lifeTime <= 0 and isAlive then 
        isAlive = false
        animator.animator:Play("shield down", false)
    end

    if not isAlive and animator.animator:HasFinished() then
        DestroyEntity(EntityID)
    end
end

function OnDestroy()
    
end

--optional use as needed
function OnCollisionEnter(other)
    
end

function OnCollisionExit(other)

end

function HandleCollision(trigger)
    
end

function OnHurt(player, damage)

end

function OnTriggerEnter(other, triggerOwner)

end

function OnTriggerExit(other)
    
end
