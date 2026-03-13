local bg = -1
local fill = -1
local hdl = -1
local sld

function Start()
    bg = GetChildren(EntityID, 0)
    fill = GetChildren(EntityID, 1)
    hdl = GetChildren(EntityID, 2)

    sld = GetSliderFrom(EntityID)
    sld.background = bg;
    sld.fill = fill;
    sld.handle = hdl;
end

function OnValueChanged()
    sld = GetSliderFrom(EntityID)
    local hdlTxt = GetChildren(hdl, 1)
    local text = GetTextFrom(hdlTxt)
    text.text = tostring(math.floor(sld.value * 100))
    setBGMVolume(sld.value)
end