function Start()
    local animator = GetAnimatorFrom(EntityID)

    local collider = GetColliderFrom(EntityID)

    if animator then
        animator.animator:Play("door idle", false)
    end

    if collider then
        collider.shapes[1].isActive = false
    end
end


function Update(dt)

    local enemiesAlive = CountEntitiesWithComponent("Enemy")
    
    if enemiesAlive <= 0 then

        local animator = GetAnimatorFrom(EntityID)

        local collider = GetColliderFrom(EntityID)

        if animator then
            animator.animator:Play("door open", false)
        end

        if collider then
            collider.shapes[1].isActive = true
        end
    end

end