function OnPointerClick()
    local canvas = GetParent(EntityID)
    local root = GetParent(canvas)
    local mainBg = GetChildren(canvas, 0)
    local playBtn = GetChildren(canvas, 1)
    local optBtn = GetChildren(canvas, 2)
    local returnBtn = GetChildren(canvas, 3)
    local optMnu = GetChildren(root, 4)

    GetButtonFrom(playBtn).interactable = false;
    GetButtonFrom(optBtn).interactable = false;
    GetButtonFrom(returnBtn).interactable = false;

    SetActiveEntity(optMnu, true)
    GetAudioComponent():Play(EntityID, "Menu Click")
end