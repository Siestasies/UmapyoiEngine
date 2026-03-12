function OnPointerClick()
    local window = GetParent(EntityID)
    local canvas = GetParent(window)
    local root = GetParent(canvas)
    local pauseMnu = GetChildren(root, 1)
    local gameOverMnu = GetChildren(root, 3)

    for i = 1, 3 do
        local button = GetChildren(pauseMnu, i)
        GetButtonFrom(button).interactable = true;
    end

    for i = 1, 3 do
        local button = GetChildren(gameOverMnu, i)
        GetButtonFrom(button).interactable = true;
    end
    
    SetActiveEntity(canvas, false)
end