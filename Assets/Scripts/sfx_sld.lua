function Start()
    local bg = GetChildren(EntityID, 0)
    local fill = GetChildren(EntityID, 1)
    local hdl = GetChildren(EntityID, 2)

    local sld = GetSliderFrom(EntityID)
    sld.background = bg;
    sld.fill = fill;
    sld.handle = hdl;
end

function OnValueChanged()
    local sld = GetSliderFrom(EntityID)
    -- sld.value
end