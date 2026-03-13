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
    LoadScene("spawn_map_v3.scn")
    GetAudioComponent():play(EntityID, "GameStart")
end