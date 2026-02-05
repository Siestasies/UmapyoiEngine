local playerEntity = -1

local maxHealth = 100
local maxMana = 100

function Start()
    -- Cross-entity access
    playerEntity = FindEntityWithComponent("Player")
    maxHealth = GetPlayerFrom(playerEntity).mMaxHealth
    maxMana = GetPlayerFrom(playerEntity).mMaxMana
end

function Update(dt)
    local health = GetPlayerFrom(playerEntity).mHealth
    local mana = GetPlayerFrom(playerEntity).mMana
    local children = GetChildrenList(EntityID)
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

        if #children >= 5 then
            local child = children[5]
            SetActiveEntity(child, false)
        end

        if #children >= 6 then
            local child = children[6]
            SetActiveEntity(child, false)
        end

        if #children >= 7 then
            local child = children[7]
            SetActiveEntity(child, false)
        end

        if #children >= 8 then
            local child = children[8]
            SetActiveEntity(child, false)
        end        

        if #children >= 9 then
            local child = children[9]
            SetActiveEntity(child, false)
        end        

        if #children >= 10 then
            local child = children[10]
            SetActiveEntity(child, false)
        end        
        
        if #children >= 11 then
            local child = children[11]
            SetActiveEntity(child, false)
        end

        if #children >= 12 then
            local child = children[12]
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

        if #children >= 5 then
            local child = children[5]
            if mana <= (maxMana * 0) then
                SetActiveEntity(child, false)
            else
                SetActiveEntity(child, true)
            end
        end

        if #children >= 6 then
            local child = children[6]
            if mana <= (maxMana * 0.2) then
                SetActiveEntity(child, false)
            else
                SetActiveEntity(child, true)
            end
        end

        if #children >= 7 then
            local child = children[7]
            if mana <= (maxMana * 0.4) then
                SetActiveEntity(child, false)
            else
                SetActiveEntity(child, true)
            end
        end

        if #children >= 8 then
            local child = children[8]
            if mana <= (maxMana * 0.6) then
                SetActiveEntity(child, false)
            else
                SetActiveEntity(child, true)
            end
        end        

        if #children >= 9 then
            local child = children[9]
            if mana <= (maxMana * 0.8) then
                SetActiveEntity(child, false)
            else
                SetActiveEntity(child, true)
            end
        end        

        if #children >= 10 then
            local child = children[10]
            SetActiveEntity(child, true)
        end        
        
        if #children >= 11 then
            local child = children[11]
            SetActiveEntity(child, true)
        end

        if #children >= 12 then
            local child = children[12]
            SetActiveEntity(child, true)
        end
    end
end