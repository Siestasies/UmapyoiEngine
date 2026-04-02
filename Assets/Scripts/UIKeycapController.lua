ExposedVars = 
{
    keyboard = "",
    controller = ""
}

local key
local ctrl
local txt

function Start()
    key = GetChildren(EntityID, 1)
    ctrl = GetChildren(EntityID, 2)
    local child = GetChildren(EntityID, 3)
    txt = GetTextFrom(child)
end

function Update(dt)
    local isController = (GetCurrentInputMethod() == 1)
    --if controller true
    if isController == false then
        SetActiveEntity(ctrl, false)
        SetActiveEntity(key, true)
        if txt then
            txt.text = keyboard
        end 
    -- if keyboard true
    else
        SetActiveEntity(ctrl, true)
        SetActiveEntity(key, false)
        if txt then
            txt.text = controller
        end 
    end
end