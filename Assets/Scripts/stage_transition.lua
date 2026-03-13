ExposedVars = {
    prefabName = "fade in.prefab",
    nextScene = "",
}

local prefabRoot = -1
local effects

function Start()
    GetAudioComponent():playFaded(EntityID, "CombatGameplayBGM", 1.5)
end

function Update(dt)

    -- simpler approach

    if effects and effects:IsClipComplete(0) then
        LoadScene(nextScene)
    end

    -- versatile approach

    -- if effects then
    --     local clip = effects.clips[1]
    --     if clip and clip.currentTime >= clip.duration and not clip.isPlaying then
    --         LoadScene(nextScene)
    --     end
    -- end
end

function OnTriggerEnter(other, triggerOwner)
    if prefabRoot ~= -1 then return end

    if HasPlayerOn(other) or HasPlayerOn(triggerOwner) then
        prefabRoot = SpawnPrefab(prefabName, Vec2(0, 0))
        if prefabRoot == -1 then
            LogError("Failed to spawn: " .. tostring(prefabName))
            return
        end

        if HasChildren(prefabRoot, 0) then
            local child = GetChildren(prefabRoot, 0)
            effects = GetEffectsFrom(child)
        end

        GetAudioComponent():play(EntityID,"SFX_Stage_Transition")
        GetAudioComponent():fadeOut(EntityID,"CombatGameplayBGM",1.5)
    end
end