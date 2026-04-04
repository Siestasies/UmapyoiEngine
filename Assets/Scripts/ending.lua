local effects
local children
local clip0totalFrames = 7
local clip1totalFrames = 18
local currFrame = 0
local currFramePlayed = false
local hasPlayedOutro = false
local hasTransitioned = false
local lastClip = false

function Start()
    effects = GetEffectsFrom(EntityID)
    children = GetChildrenList(EntityID)
end

function Update(dt)
    local parent = GetParent(EntityID)
    local gm = GetParent(parent)

    if GetActiveEntity(parent) and not hasPlayedOutro then
        hasPlayedOutro = true
        GetAudioComponent():stop(gm, "CombatGameplayBGM")
        GetAudioComponent():playFaded(EntityID, "BGM_Outro", 1.5, true)
    end

    if effects and not hasTransitioned
       and effects:IsClipComplete(0)
       and effects:IsClipComplete(1) then
        hasTransitioned = true
        SetActiveEntity(children[1], true)
        SetActiveEntity(children[2], true)
        GetAudioComponent():stop(EntityID, "BGM_Outro")
        GetAudioComponent():playFaded(gm, "CombatGameplayBGM", 1.5, true)
    end

    -- Derive frame from the effect's own progress instead of a separate timer
    local clip = effects.clips[1]  -- index 1 in Lua (the clip at index 0)
    local progress = clip:GetProgress()  -- 0.0 to 1.0
    local frame = math.floor(progress * clip0totalFrames) + 1

    if frame ~= currFrame then
        currFrame = frame
        currFramePlayed = false
    end

    if not currFramePlayed then
        --if not audio then return end

        local soundName = "Cutscene_End" .. currFrame

        Log("curr sound : " .. soundName)

        GetAudioComponent():play(EntityID, soundName)

        currFramePlayed = true
    end

    --for clip one 
    if effects:IsClipComplete(0) and not effects:IsClipComplete(1) then
        -- Derive frame from the effect's own progress instead of a separate timer
        -- local clip = effects.clips[1]  -- index 1 in Lua (the clip at index 0)
        -- local progress = clip:GetProgress()  -- 0.0 to 1.0
        -- local frame = math.floor(progress * clip1totalFrames) + 1

        -- if frame ~= currFrame then
        --     currFrame = frame
        --     currFramePlayed = false
        -- end

        -- if not currFramePlayed then
        --     --if not audio then return end

        --     local soundName = "Cutscene_End" .. currFrame + 7

        --     Log("curr sound : " .. soundName)

        --     GetAudioComponent():play(EntityID, soundName)

        --     currFramePlayed = true
        -- end
        if lastClip == false then
            GetAudioComponent():play(EntityID, "Cutscene_End8")
            lastClip = true
        end
    end
end