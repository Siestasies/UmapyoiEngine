function Update(dt)
    local children = GetChildrenList(EntityID)

    local paused = IsGamePause()

    if paused == true then
        toggleGroupLowPass("MASTER", true)

        if #children > 0 then
            local child = children[1]
            SetActiveEntity(child, true)
        end

        if #children >= 2 then
            local child = children[2]
            SetActiveEntity(child, true)
        end

        if #children >= 3 then
            local child = children[3]
            SetActiveEntity(child, true)
        end

        if #children >= 4 then
            local child = children[4]
            SetActiveEntity(child, true)
        end

    else
        toggleGroupLowPass("MASTER", false)

        if #children > 0 then
            local child = children[1]
            SetActiveEntity(child, false)
        end

        if #children >= 2 then
            local child = children[2]
            SetActiveEntity(child, false)
        end

        if #children >= 3 then
            local child = children[3]
            SetActiveEntity(child, false)
        end

        if #children >= 4 then
            local child = children[4]
            SetActiveEntity(child, false)
        end
    end
end