function Start()
    local bg = GetChildren(EntityID, 0)
    local cmk = GetChildren(EntityID, 1)

    local cbx = GetCheckboxFrom(EntityID)
    cbx.background = bg;
    cbx.checkmark = cmk;
end

function OnToggle()
    local cbx = GetCheckboxFrom(EntityID)
    -- cbx.isChecked
end