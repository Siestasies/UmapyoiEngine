local start
function Start()
    start = false
end

function Update(dt)
    if start == false then
        --audio:play(EntityID, "MainMenuBGM")
        PlaySound("MainMenuBGM", 0.2, -1)
        start = true
    end
end