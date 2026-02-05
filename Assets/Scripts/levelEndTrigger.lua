local levelEndTrigger = {
    levelEnd = false
}

function levelEndTrigger.getLevelEnd()
    return levelEndTrigger.levelEnd
end

function levelEndTrigger.setLevelEnd(x)
    levelEndTrigger.levelEnd = x
end

return levelEndTrigger