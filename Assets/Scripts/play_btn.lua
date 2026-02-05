local time = 0
local clicked = false
local image
local fadeState

function Start()
    fadeState = require("fadeState")
end

function OnPointerEnter()
    local effects = GetEffects()
    effects:Play("HoverEnter")
end

function OnPointerExit()
    local effects = GetEffects()
    effects:StopAll()
    effects:Play("HoverExit")
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
            LoadScene("tutorial.scn")
        end
    end
end