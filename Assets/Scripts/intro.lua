local effects

function Start()
    local frames = GetChildren(EntityID, 0)
    effects = GetEffectsFrom(frames)
end

function Update(dt)
    if effects and effects:IsClipComplete(0) then
        LoadScene("main_menu.scn")
    end
end
