local levelEndTrigger 
local fadeState
local time = 1
local loadNextScene = false

ExposedVars = {
    nextSceneName = "test_combat"
}

function OnTriggerEnter(other)
    Log("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX PLAYER HIT PLAYER HIT")
    if HasPlayerOn(other) then
        loadNextScene = true
    end
end

function Start()
    levelEndTrigger = require("levelEndTrigger")
    fadeState = require("fadeState")
    levelEndTrigger.setLevelEnd(false)
end

function Update(dt)
    local enemies = FindEntitiesWithComponent("Enemy")
    if not enemies then
        levelEndTrigger.setLevelEnd(true)
    end

    -- timer for when want to load next scene
    if loadNextScene then
        time = time - dt
        if time <= 0 then
            time = 1
            LoadScene(nextSceneName .. ".scn")
        end
    end
end