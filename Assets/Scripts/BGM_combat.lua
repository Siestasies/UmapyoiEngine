local start
local audio = nil
function Start()
    start = false
end

function Update(dt)
    if start == false then
        PlaySound("CombatGameplayBGM", 0.1, -1); 
        --audio = GetAudioComponent()
        --audio:play(EntityID, "CombatGameplayBGM")
        start = true
    end
end