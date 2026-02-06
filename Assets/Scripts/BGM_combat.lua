local start
local audio = nil
function Start()
    start = false
end

function Update(dt)
    if start == false then
        local audio = GetAudioComponent()
        audio:playFaded(EntityID, "CombatGameplayBGM", 1.5)
        start = true
    end
end