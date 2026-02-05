-- local levelEndState
-- local fadeState
-- local time = 1
-- local loadNextScene = false

function Start()
    
end

function Update(dt)
end

-- when player step on trigger
function OnTriggerEnter(other, triggerOwner)
    -- trigger fading
   
    if HasPlayerOn(triggerOwner) or HasPlayerOn(other) then

        --Log("NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN : " .. other)
        
        local enemyCount = CountEntitiesWithComponent("Enemy")

        if enemyCount <= 0 then
            LoadScene("tutorial.scn")
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