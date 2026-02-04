local levelEndState = {
    levelEnd = false,
    levelEndMenu = false
}

function levelEndState.getLevelEnd()
    return levelEndState.levelEnd
end

function levelEndState.setLevelEnd(x)
    levelEndState.levelEnd = x
end

function levelEndState.getLevelEndMenu()
    return levelEndState.levelEndMenu
end

function levelEndState.setLevelEndMenu(x)
    levelEndState.levelEndMenu = x
end

return levelEndState