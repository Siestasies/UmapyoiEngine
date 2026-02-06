local start
local audio
function Start()
    start = false
    audio = GetAudioComponent()
end

function Update(dt)
    if start == false then
        audio:playFaded(EntityID, "CombatGameplayBGM", 3)
        start = true
    end
end