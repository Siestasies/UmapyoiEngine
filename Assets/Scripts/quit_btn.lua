local time = 0
local clicked = false

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