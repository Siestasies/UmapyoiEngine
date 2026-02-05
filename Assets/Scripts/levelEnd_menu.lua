local levelEndState

function Start()
    levelEndState = require("levelEndState")
    levelEndState.setLevelEndMenu(false)
end

function Update()
    if levelEndState.getLevelEndMenu() then
        --PauseGame(true)
        local children = GetChildrenList(EntityID)
        local child = children[1]
        -- show end menu
        SetActiveEntity(child, true)
    end
end