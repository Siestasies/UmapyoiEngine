local levelEndState = {
    levelEnd = false
}

function levelEndState.getLevelEnd()
    return levelEndState.levelEnd
end

function levelEndState.setLevelEnd(x)
    levelEndState.levelEnd = x
end

return levelEndState