local start
function Start()
    start = false
end

function Update(dt)
    if start == false then
        GetAudioComponent():playFaded(EntityID, "CombatGameplayBGM", 1.5)
        start = true
    end
end