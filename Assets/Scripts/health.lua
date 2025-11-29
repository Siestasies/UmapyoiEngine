local playerEntity = -1

local maxHealth = 100

function Start()
    -- Global utilities (same for everyone)
    Log("Script started at: " .. GetDeltaTime())
    
    -- Entity-specific context
    Log("My entity ID: " .. EntityID)

    thisEntity = GetEntity(EntityID)
    
    -- Cross-entity access
    playerEntity = FindEntityWithComponent("Player")
    Log("Player: " .. playerEntity)

    local maxHealth = GetPlayerFrom(playerEntity).mHealth

end

function Update(dt)
    local health = GetPlayerFrom(playerEntity).mHealth

    local children = GetChildren(EntityID)

    if #children > 0 then
        local child = children[1]
        Log("Children 1: " .. child)
        if health <= (maxHealth * 0) then
            SetActiveEntity(child, false)
        else
            SetActiveEntity(child, true)
        end
    end

    if #children >= 2 then
        local child = children[2]
        Log("Children 2: " .. child)        
        if health <= (maxHealth * 0.25) then
            SetActiveEntity(child, false)
        else
            SetActiveEntity(child, true)
        end
    end

    if #children >= 3 then
        local child = children[3]
        Log("Children 3: " .. child)        
        if health <= (maxHealth * 0.5) then
            SetActiveEntity(child, false)
        else
            SetActiveEntity(child, true)
        end
    end

    if #children >= 4 then
        local child = children[4]
        Log("Children 4: " .. child)
        if health <= (maxHealth * 0.75) then
            SetActiveEntity(child, false)
        else
            SetActiveEntity(child, true)
        end
    end
end