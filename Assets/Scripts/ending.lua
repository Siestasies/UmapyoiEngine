local effects
local children
local clip0totalFrames = 7
local clip1totalFrames = 18
local currFrame = 0
local currFramePlayed = false

function Start()
    effects = GetEffectsFrom(EntityID)
    children = GetChildrenList(EntityID)
    GetAudioComponent():play(EntityID, "BGM_Outro")
end

function Update(dt)
    if effects and (effects:IsClipComplete(0) and effects:IsClipComplete(1)) then
        SetActiveEntity(children[1], true)
        SetActiveEntity(children[2], true)
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
        GetAudioComponent():play(EntityID, "Cutscene_End8")
    end
end