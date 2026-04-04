    local effects
    --local audio
    local totalFrames = 10  -- adjust to your actual frame count
    local currFrame = 0
    local currFramePlayed = false

    function Start()
        local frames = GetChildren(EntityID, 0)
        effects = GetEffectsFrom(frames)
        --audio = GetAudioComponent()
        GetAudioComponent():play(EntityID, "BGM_Intro")
    end

    function Update(dt)
        if not effects then return end

        -- Derive frame from the effect's own progress instead of a separate timer
        local clip = effects.clips[1]  -- index 1 in Lua (the clip at index 0)
        local progress = clip:GetProgress()  -- 0.0 to 1.0
        local frame = math.floor(progress * totalFrames) + 1

        if frame ~= currFrame then
            currFrame = frame
            currFramePlayed = false
        end

        if effects:IsClipComplete(0) then
            LoadScene("main_menu.scn")
            return
        end

        if not currFramePlayed then
            --if not audio then return end

            local soundName = "Cutscene_Frame_" .. currFrame

            Log("curr sound : " .. soundName)

            GetAudioComponent():play(EntityID, soundName)

            currFramePlayed = true
        end
    end