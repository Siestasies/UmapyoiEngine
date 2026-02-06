local isShown = false

function Update(dt)
    local children = GetChildrenList(EntityID)

    local playerEntity = FindEntityWithComponent("Player")
    if not IsEntityValid(playerEntity) then return end

    local player = GetPlayerFrom(playerEntity)
    if not player then return end

    if player.mHealth <= 50 then
        if #children > 0 then
            SetActiveEntity(children[1], true)
        end
        if #children >= 2 then
            SetActiveEntity(children[2], true)
        end
        if #children >= 3 then
            SetActiveEntity(children[3], true)
        end
    else
        if #children > 0 then
            SetActiveEntity(children[1], false)
        end
        if #children >= 2 then
            SetActiveEntity(children[2], false)
        end
        if #children >= 3 then
            SetActiveEntity(children[3], false)
        end
    end
end
