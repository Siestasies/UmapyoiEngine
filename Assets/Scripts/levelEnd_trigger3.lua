local levelEnd
local fadeState
local time = 1

function Start()
    levelEnd = false
    fadeState = require("fadeState")
    levelEndState = require("levelEndState")
end

function Update(dt)
    if levelEnd then
        levelEndState.setLevelEndMenu(true)
    end
end

-- when player step on trigger
function OnTriggerEnter(other, triggerOwner)
    -- trigger fading
    if HasPlayerOn(triggerOwner) or HasPlayerOn(other) then
        local enemyCount = CountEntitiesWithComponent("Enemy")
        if enemyCount <= 0 then
            levelEnd = true
        end
    end
end