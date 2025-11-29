local playerEntity = -1

local maxHealth = 100

function Start()
    -- Cross-entity access
    playerEntity = FindEntityWithComponent("Player")
    maxHealth = GetPlayerFrom(playerEntity).mHealth
end

function Update(dt)

    local health = GetPlayerFrom(playerEntity).mHealth
    local children = GetChildren(EntityID)
    local paused = IsGamePause()

    if paused == true then
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
    else
        if #children > 0 then
            local child = children[1]
            if health <= (maxHealth * 0) then
                SetActiveEntity(child, false)
            else
                SetActiveEntity(child, true)
            end
        end

        if #children >= 2 then
            local child = children[2]
            if health <= (maxHealth * 0.25) then
                SetActiveEntity(child, false)
            else
                SetActiveEntity(child, true)
            end
        end

        if #children >= 3 then
            local child = children[3]
            if health <= (maxHealth * 0.5) then
                SetActiveEntity(child, false)
            else
                SetActiveEntity(child, true)
            end
        end

        if #children >= 4 then
            local child = children[4]
            if health <= (maxHealth * 0.75) then
                SetActiveEntity(child, false)
            else
                SetActiveEntity(child, true)
            end
        end
    end
end