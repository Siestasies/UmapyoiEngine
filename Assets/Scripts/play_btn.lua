local time = 0
local clicked = false
local image
local fadeState

function Start()
    fadeState = require("fadeState")
end

function OnPointerEnter()
    if HasEffects() then
        local effects = GetEffects()
        effects:Play("HoverEnter")
    end
end

function OnPointerExit()
    if HasEffects() then
        local effects = GetEffects()
        effects:StopAll()
        effects:Play("HoverExit")
    end
end


function OnClick()
    PlaySound("startbtn_sound", 0.8, 0)
    fadeState.setFading(true)
    clicked = true
end

function Update(dt)
    -- timer 1 sec to let fade first
    if clicked then
        time = time + dt
        if time >= 1 then
            time = 0
            clicked = false
            LoadScene("spawn_map_v2.scn")
        end
    end
end