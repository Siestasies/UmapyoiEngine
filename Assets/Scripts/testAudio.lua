local audio = nil

function Start()
    if HasAudioComponent() then
        audio = GetAudioComponent()
    end
end

function Update(dt)
    if KeyReleased(KEY_0) then
        if HasAudioComponent("explosion") then
            Log("slay")
            audio:play(EntityID,"explosion")
        end
    end
end