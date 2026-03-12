local playerEntity = -1
local levelEndState

function Start()
    playerEntity = FindEntityWithComponent("Player")
    levelEndState = require("levelEndState")    
end

function Update(dt)
    local children = GetChildrenList(EntityID)
    
    local player = GetPlayerFrom(playerEntity)
    local paused = IsGamePause()
    if paused == true and player.mHealth > 0 then
        toggleGroupLowpass("MASTER", true)

        if #children >= 2 then
            SetActiveEntity(children[2], true) -- pause
        end
        return
    else
        toggleGroupLowpass("MASTER", false)

        if #children >= 2 then
            SetActiveEntity(children[2], false)
    end

    if IsEntityValid(playerEntity) then
        if player.mHealth <= 0 then
            if #children >= 2 then
                SetActiveEntity(children[2], false)
            end
            if #children >= 3 then
                SetActiveEntity(children[3], false)
            end
            if #children >= 4 then
                SetActiveEntity(children[4], true) -- game over
            end
            if #children >= 5 then
                SetActiveEntity(children[5], false)
            end
            return
        end
    end

    if levelEndState.getLevelEnd() then
        if #children >= 3 then
            SetActiveEntity(children[3], true) -- complete
        else
            SetActiveEntity(children[3], false)
        end
    end
end