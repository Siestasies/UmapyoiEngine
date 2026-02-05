local time = 0
local clicked = false

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
    PlaySound("quitbtn_sound", 1.0, 0)
    clicked = true
end

function Update(dt)
    if clicked then
        time = time + dt
        if time > 1 then
            time = 0
            clicked = false
            CloseApplication()
        end
    end
end