function OnPointerEnter()
    local child = GetChildren(EntityID, 0)
    local text = GetTextFrom(child)
    text.color.r = 0.953
    text.color.g = 0.859
    text.color.b = 0.757
    text.color.a = 1.0
end

function OnPointerExit()
    local child = GetChildren(EntityID, 0)
    local text = GetTextFrom(child)
    text.color.r = 0.792
    text.color.g = 0.651
    text.color.b = 0.584
    text.color.a = 1.0
end

function OnPointerClick()
    local canvas = GetParent(EntityID)
    local playBtn = GetChildren(canvas, 1)
    local helpBtn = GetChildren(canvas, 2)
    local credBtn = GetChildren(canvas, 3)
    local quitBtn = GetChildren(canvas, 4)
    local cfmMnu = GetChildren(canvas, 7)

    GetButtonFrom(playBtn).interactable = false;
    GetButtonFrom(helpBtn).interactable = false;
    GetButtonFrom(credBtn).interactable = false;
    GetButtonFrom(quitBtn).interactable = false;

    SetActiveEntity(cfmMnu, true)
    GetAudioComponent():play(EntityID, "Menu Click")
end