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

    if KeyReleased(KEY_9) then
        if HasAudioComponent("hurt") then
            Log("slay")
            audio:playOneShot(EntityID,"hurt")
        end
    end

    if KeyReleased(KEY_8) then
        if HasAudioComponent("explosion") then
            Log("slay")
            audio:stop(EntityID,"explosion")
        end
    end
end