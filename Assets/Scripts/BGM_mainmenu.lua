local start
local audio = nil
function Start()
    start = false
    audio = GetAudioComponent()
end

function Update(dt)
    if start == false then
        audio:playFaded(EntityID, "MainMenuBGM", 1.5)
        start = true
    end
    if KeyDown(KEY_P) then
        audio:toggleLowpass(EntityID, "MainMenuBGM", false)
    elseif KeyDown(Key_O) then
        audio:toggleLowpass(EntityID, "MainMenuBGM", true)
    end
end