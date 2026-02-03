local levelEndState
local fadeState
local time = 1
local loadNextScene = false

ExposedVars = {
    nextSceneName = "test_default2"
}

-- when player step on trigger
function OnTriggerEnter(other)
    Log("[LOG] PLAYER HIT PLAYER HIT")
    -- trigger fading
    if HasPlayerOn(other) then
        loadNextScene = true
        fadeState.setFading(true)
    end
end

function OnCollision(other)
    Log("[LOG] WHAT HIT")
end

function Start()
    levelEndState = require("levelEndState")
    fadeState = require("fadeState")
    levelEndState.setLevelEnd(false)
end

function Update(dt)
    -- if no enemies, set level end flag
    local enemies = FindEntitiesWithComponent("Enemy")
    if enemies then
        levelEndState.setLevelEnd(true)
    end

    -- timer for when want to load next scene
    -- after player step on trigger
    if loadNextScene then
        time = time - dt
        if time <= 0 then
            time = 1
            LoadScene(nextSceneName .. ".scn")
        end
    end
end