local levelEnd
local fadeState
local time = 1

function Start()
    levelEnd = false
    fadeState = require("fadeState")
end

function Update(dt)
    if levelEnd then
        fadeState.setFading(true)
        time = time - dt
        if time <= 0 then 
            time = 0
            LoadScene("tutorial_v2.scn")
        end
    end
end

-- when player step on trigger
function OnTriggerEnter(other, triggerOwner)
    -- trigger fading
    if HasPlayerOn(triggerOwner) or HasPlayerOn(other) then
        local enemyCount = CountEntitiesWithComponent("Enemy")
        if enemyCount <= 0 then
            levelEnd = true
            GetAudioComponent():fadeOut(EntityID,"MainMenuBGM",1.5)
        end
    end
end

--function Start()
--    levelEndState = require("levelEndState")
--    fadeState = require("fadeState")
--    levelEndState.setLevelEnd(false)
--end
--
--function Update(dt)
--    -- if no enemies, set level end flag
--    local enemies = FindEntitiesWithComponent("Enemy")
--    -- set to 1 for debugging
--    if enemies <= 0 then
--        levelEndState.setLevelEnd(true)
--    end
--
--    -- timer for when want to load next scene
--    -- after player step on trigger
--    if loadNextScene then
--        fadeState.setFading(true)
--        time = time - dt
--        if time <= 0 then
--            time = 1
--            LoadScene("tutorial.scn")
--        end
--    end
--end