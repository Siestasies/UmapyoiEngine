local active = false;

function Update(dt)
    if KeyPressed(KEY_F3) then
        active = not active;
    end

    local textComponent = GetTextFrom(EntityID)
    if active == true then
        local fps = GetFps()
        textComponent.text = tostring(math.floor(fps))
    else
        textComponent.text = ""
    end
end