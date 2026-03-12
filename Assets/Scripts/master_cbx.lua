local originalVolume

function Start()
    local bg = GetChildren(EntityID, 0)
    local cmk = GetChildren(EntityID, 1)

    local cbx = GetCheckboxFrom(EntityID)
    cbx.background = bg;
    cbx.checkmark = cmk;

    originalVolume = 0
end

function OnToggle()
    local cbx = GetCheckboxFrom(EntityID)
    -- cbx.isChecked
    if cbx.isChecked == false then
        originalVolume = getMasterVolume()
        setMasterVolume(0)
    else
        setMasterVolume(originalVolume)
    end
end