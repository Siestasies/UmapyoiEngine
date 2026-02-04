local image
local fadeState
local time = 1

function Start()
    fadeState = require("fadeState")
    image = GetImage()
end

-- kinda reatrded but
-- start of scene time = 1, MINUS dt, fade out time = 0
-- when getFading switched, time = 0, PLUS dt, fade into time = 1
function Update(dt)
    -- fade FROM black (at start of scene)
    if fadeState.getFading() == false then
        time = time - dt
        image.color.a = time;
        if time <= 0 then
            time = 0
            clicked = false
            image.color.a = 0.0
        end
    end

    -- fade TO black (at end of scene/ load another scene)
    if fadeState.getFading() then
        time = time + dt
        image.color.a = time;
        if time >= 1 then
            time = 0
            image.color.a = 1.0
        end
    end
end