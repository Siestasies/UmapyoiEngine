local levelEndState
local children

function Start()
    levelEndState = require("levelEndState")
end

function Update(dt)
    children = GetChildrenList(EntityID)

    local paused = IsGamePause()
    local gameEnd = levelEndState.getLevelEnd()

    if paused == true and gameEnd == false then
        SetShowPauseMenu(true)
    elseif paused == true and gameEnd == true then
        -- do nothing
    else
        SetShowPauseMenu(false)
    end
end

function SetShowPauseMenu(b)
    if #children > 0 then
        local child = children[1]
        SetActiveEntity(child, b)
    end

    if #children >= 2 then
        local child = children[2]
        SetActiveEntity(child, b)
    end

    if #children >= 3 then
        local child = children[3]
        SetActiveEntity(child, b)
    end

    if #children >= 4 then
        local child = children[4]
        SetActiveEntity(child, b)
    end
end