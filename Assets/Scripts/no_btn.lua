function OnPointerClick()
    local cfmMnu = GetParent(EntityID)
    local canvas = GetParent(cfmMnu)
    local playBtn = GetChildren(canvas, 1)
    local helpBtn = GetChildren(canvas, 2)
    local credBtn = GetChildren(canvas, 3)
    local quitBtn = GetChildren(canvas, 4)

    GetButtonFrom(playBtn).interactable = true;
    GetButtonFrom(helpBtn).interactable = true;
    GetButtonFrom(credBtn).interactable = true;
    GetButtonFrom(quitBtn).interactable = true;

    SetActiveEntity(cfmMnu, false)
end