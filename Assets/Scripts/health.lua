--basically include
ExposedVars = {
    isActive = true
}

local playerEntity = -1

function Start()
    -- Global utilities (same for everyone)
    Log("Script started at " .. GetDeltaTime())
    
    -- Entity-specific context
    Log("My entity ID: " .. EntityID)

    thisEntity = GetEntity(EntityID)
    
    local myTransform = GetTransform(EntityID)
    if myTransform then
        Log("I'm active?: " .. myTransform.isActive)
    end
    
    -- Cross-entity access
    playerEntity = FindEntityWithComponent("Player")
    Log("player : " .. playerEntity)
    
end

function Update(dt)
    local health = GetPlayerFrom(playerEntity).mHealth

    local children = GetChildren(thisEntity)

    if #children > 0 then
        local child = children[1]
        if health >= health * 0 then
            SetActiveEntity(child, false)
        else
            SetActiveEntity(child, true)
        end
    end

    if #children >= 2 then
        local child = children[2]
        if health >= health * 0.25 then
            SetActiveEntity(child, false)
        else
            SetActiveEntity(child, true)
        end
    end

    if #children >= 3 then
        local child = children[3]
        if health >= health * 0.5 then
            SetActiveEntity(child, false)
        else
            SetActiveEntity(child, true)
        end
    end

    if #children >= 4 then
        local child = children[4]
        if health >= health * 0.75 then
            SetActiveEntity(child, false)
        else
            SetActiveEntity(child, true)
        end
    end


end