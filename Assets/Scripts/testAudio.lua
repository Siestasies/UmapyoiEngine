local audio = nil

function Start()
    if HasAudioComponent() then
        audio = GetAudioComponent()
    end
end

function Update(dt)
    -- KEY_0: Play explosion normally
    if KeyReleased(KEY_0) then
        if HasAudioComponent("explosion") then
            Log("Play explosion (normal)")
            audio:play(EntityID, "explosion")
        end
    end

    -- KEY_9: Play hurt one-shot normally
    if KeyReleased(KEY_9) then
        if HasAudioComponent("hurt") then
            Log("Play hurt one-shot (normal)")
            audio:playOneShot(EntityID, "hurt")
        end
    end

    -- KEY_1: FADE IN explosion (2 seconds)
    if KeyReleased(KEY_1) then
        if HasAudioComponent("cave") then
            Log("Fade IN explosion (2s)")
            audio:playFaded(EntityID, "cave", 2.0)
        end
    end

    -- KEY_8: INSTANT stop explosion
    if KeyReleased(KEY_8) then
        if HasAudioComponent("explosion") then
            Log("INSTANT stop explosion")
            audio:stop(EntityID, "explosion")
        end
    end

    -- KEY_2: FADE OUT explosion (1 second)
    if KeyReleased(KEY_2) then
        if HasAudioComponent("cave") then
            Log("FADE OUT explosion (1s)")
            audio:fadeOut(EntityID, "cave", 5.0)
        end
    end

    -- KEY_3: FADE OUT ALL sounds on entity (1.5 seconds)
    if KeyReleased(KEY_3) then
        Log("FADE OUT ALL sounds (1.5s)")
        audio:fadeOut(EntityID, 1.5)
    end

    -- KEY_4: Play hurt one-shot with quick fade-in (0.3s)
    if KeyReleased(KEY_4) then
        if HasAudioComponent("hurt") then
            Log("One-shot with quick fade-in (0.3s)")
            audio:playFaded(EntityID, "hurt", 0.3)
        end
    end
end
