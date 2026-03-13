--[[
\file   CutsceneTrigger.lua
\par    Project: GAM250
\par    Course: CSD2401
\par    Section A

\author Javier Chua Dong Qing (100%)
\par    E-mail: javierdongqing.chua@digipen.edu
\par    DigiPen login: javierdongqing.chua

    Attach to: any entity with Collider (Trigger), Cutscene, and Dialogue components.

    Activates a cutscene when the player enters the trigger collider.
    Reads actions from the Cutscene component and executes them in order:
      - SetCameraToPosition (0): snap camera to a position
      - LerpCameraToPosition (1): smoothly move camera over duration
      - PlayDialogue (2): play a dialogue sequence (uses Dialogue component)
      - Wait (3): pause for duration seconds
      - ReturnCameraToPlayer (4): re-enable camera follow
      - ShakeCamera (5): trigger screen shake with intensity/duration
      - LerpCameraZoom (6): smoothly lerp camera zoom to targetZoom over duration

    Requires Dialogue.prefab for dialogue display.

    ExposedVars:
        prefabName - dialogue prefab file name (include .prefab extension)
--]]

-- ExposedVars
ExposedVars = {
    prefabName = "Dialogue.prefab",
}

prefabName = "Dialogue.prefab"

-- Action type constants (must match CutsceneActionType enum)
local ACT_SET_CAMERA    = 0
local ACT_LERP_CAMERA   = 1
local ACT_PLAY_DIALOGUE = 2
local ACT_WAIT          = 3
local ACT_RETURN_CAMERA = 4
local ACT_SHAKE_CAMERA  = 5
local ACT_LERP_ZOOM     = 6

-- Dialogue config
local CHARS_PER_SECOND = 40
local BLINK_PERIOD     = 0.5

-- Prefab entity handles (for dialogue display)
local dialogueRoot = -1
local ePanel       = -1
local eTextField   = -1
local eName        = -1
local eNameField   = -1
local ePortrait    = -1
local eContinue    = -1

-- Cutscene state
local isPlaying     = false
local actions       = {}
local actionIndex   = 0
local cameraEntity  = -1
local ownerEntity   = -1  -- the entity that owns this cutscene

-- Per-action state
local lerpStartX    = 0
local lerpStartY    = 0
local lerpTargetX   = 0
local lerpTargetY   = 0
local lerpTimer     = 0
local lerpDuration  = 1

local zoomStart     = 10
local zoomTarget    = 10
local zoomTimer     = 0
local zoomDuration  = 1

local waitTimer     = 0

-- Dialogue state (when playing a dialogue action)
local dialogueActive = false
local dialogueLines  = {}
local lineIndex      = 0
local fullText       = ""
local revealed       = 0.0
local isTyping       = false
local blinkTimer     = 0.0
local blinkOn        = true
local skipFrames     = 0
local cooldownTimer  = 0

-- Helpers
local function ShowImg(e, visible)
    if e == -1 then return end
    SetActiveEntity(e, visible)
end

local function ShowTxt(e, visible)
    if e == -1 then return end
    local t = GetTextFrom(e)
    if t then t.visible = visible end
end

local function SetBodyText(str)
    local t = GetTextFrom(eTextField)
    if t then t.text = str end
end

local function SetNameText(str)
    local t = GetTextFrom(eNameField)
    if t then t.text = str end
end

local function ApplyLine(line)
    local hasSpeaker  = line.speaker  and line.speaker  ~= ""
    local hasPortrait = line.portrait and line.portrait ~= ""

    ShowImg(eName,      hasSpeaker)
    ShowTxt(eNameField, hasSpeaker)
    if hasSpeaker then SetNameText(line.speaker) end

    ShowImg(ePortrait, hasPortrait)
    if hasPortrait then
        local img = GetImageFrom(ePortrait)
        if img then img.textureName = line.portrait end
    end

    fullText   = line.text or ""
    revealed   = 0.0
    isTyping   = true
    blinkTimer = 0.0
    blinkOn    = true

    SetBodyText("")
    ShowImg(eContinue, false)
end

local function FinishTyping()
    revealed = #fullText
    isTyping = false
    SetBodyText(fullText)
    ShowImg(eContinue, true)
end

local function CloseDialogue()
    dialogueActive = false
    dialogueLines  = {}
    lineIndex      = 0
    isTyping       = false
    if dialogueRoot ~= -1 then
        SetActiveEntity(dialogueRoot, false)
    end
end

local function AdvanceDialogueLine()
    lineIndex = lineIndex + 1
    if lineIndex > #dialogueLines then
        CloseDialogue()
    else
        ApplyLine(dialogueLines[lineIndex])
    end
end

local function OpenDialogue(seqId)
    if dialogueRoot == -1 then
        Log("[CutsceneTrigger] Cannot play dialogue: dialogue prefab not spawned")
        dialogueActive = false
        return
    end

    -- Read dialogue data from the cutscene owner entity (not the prefab)
    local seq = GetDialogueSequence(ownerEntity, seqId)
    if not seq or #seq == 0 then
        Log("[CutsceneTrigger] Dialogue sequence not found: " .. tostring(seqId))
        dialogueActive = false
        return
    end

    dialogueLines  = seq
    lineIndex      = 1
    skipFrames     = 2
    dialogueActive = true

    SetActiveEntity(dialogueRoot, true)
    ApplyLine(dialogueLines[1])
end

-- Spawn the dialogue prefab (only when cutscene triggers, not in Start)
local function SpawnDialoguePrefab()
    if dialogueRoot ~= -1 then return true end

    dialogueRoot = SpawnPrefab(prefabName, Vec2(0, 0))

    if dialogueRoot == -1 then
        LogError("[CutsceneTrigger] Failed to spawn: " .. tostring(prefabName))
        return false
    end

    ePanel    = GetChildren(dialogueRoot, 0)
    eName     = GetChildren(dialogueRoot, 1)
    ePortrait = GetChildren(dialogueRoot, 2)
    eContinue = GetChildren(dialogueRoot, 3)

    if ePanel ~= -1 then
        eTextField = GetChildren(ePanel, 0)
    end
    if eName ~= -1 then
        eNameField = GetChildren(eName, 0)
    end

    SetActiveEntity(dialogueRoot, false)
    return true
end

-- Destroy the dialogue prefab (cleanup after cutscene finishes)
local function DestroyDialoguePrefab()
    if dialogueRoot ~= -1 then
        DestroyWithChildren(dialogueRoot)
        dialogueRoot = -1
        ePanel       = -1
        eTextField   = -1
        eName        = -1
        eNameField   = -1
        ePortrait    = -1
        eContinue    = -1
    end
end

-- Forward declarations for mutual recursion
local NextAction
local BeginAction

-- Move to next action in the cutscene
NextAction = function()
    actionIndex = actionIndex + 1
    if actionIndex > #actions then
        -- Cutscene finished
        isPlaying = false
        actionIndex = 0
        cooldownTimer = 1.0  -- prevent immediate re-trigger from OnTrigger
        SetCutsceneActive(false)
        SetCutscenePlayed(ownerEntity, true)
        DestroyDialoguePrefab()
        Log("[CutsceneTrigger] Cutscene finished on entity " .. tostring(EntityID))
        return
    end
    BeginAction()
end

-- Start the current action
BeginAction = function()
    local action = actions[actionIndex]
    if not action then
        isPlaying = false
        return
    end

    local aType = action.type

    if aType == ACT_SET_CAMERA then
        SetCameraFollow(cameraEntity, false)
        SetCameraPosition(cameraEntity, action.targetX, action.targetY)
        NextAction()

    elseif aType == ACT_LERP_CAMERA then
        SetCameraFollow(cameraEntity, false)
        lerpStartX, lerpStartY = GetTransformPosition(cameraEntity)
        lerpTargetX = action.targetX
        lerpTargetY = action.targetY
        lerpDuration = action.duration
        lerpTimer = 0
        if lerpDuration <= 0 then
            SetCameraPosition(cameraEntity, lerpTargetX, lerpTargetY)
            NextAction()
            return
        end

    elseif aType == ACT_PLAY_DIALOGUE then
        OpenDialogue(action.dialogueSequenceId)
        if not dialogueActive then
            NextAction()
        end

    elseif aType == ACT_WAIT then
        waitTimer = action.duration

    elseif aType == ACT_LERP_ZOOM then
        zoomStart = GetCameraZoom(cameraEntity)
        zoomTarget = action.targetZoom
        zoomDuration = action.duration
        zoomTimer = 0
        if zoomDuration <= 0 then
            SetCameraZoom(cameraEntity, zoomTarget)
            NextAction()
            return
        end

    elseif aType == ACT_RETURN_CAMERA then
        SetCameraZoom(cameraEntity, 10)
        SetCameraFollow(cameraEntity, true)
        NextAction()

    elseif aType == ACT_SHAKE_CAMERA then
        CameraShake(cameraEntity, action.shakeIntensity, action.duration)
        NextAction()
    end
end

-- Lifecycle

function Start()
    -- Don't spawn dialogue prefab here - spawn lazily when cutscene triggers
    -- This avoids having N dialogue prefabs alive at once for N cutscene entities
    cameraEntity = FindCameraEntity()
    Log("[CutsceneTrigger] Ready. Entity=" .. tostring(EntityID) .. " Camera=" .. tostring(cameraEntity))
end

function Update(dt)
    if cooldownTimer > 0 then
        cooldownTimer = cooldownTimer - dt
    end
    if not isPlaying then
        -- Ensure camera follows the player only when no cutscene is active globally
        if not IsCutsceneActive() and cameraEntity ~= -1 then
            SetCameraFollow(cameraEntity, true)
        end
        return
    end

    local action = actions[actionIndex]
    if not action then return end

    local aType = action.type

    -- Lerp camera
    if aType == ACT_LERP_CAMERA then
        lerpTimer = lerpTimer + dt
        local t = lerpTimer / lerpDuration
        if t >= 1.0 then
            t = 1.0
            SetCameraPosition(cameraEntity, lerpTargetX, lerpTargetY)
            NextAction()
        else
            -- Ease out quad
            t = t * (2.0 - t)
            local cx = lerpStartX + (lerpTargetX - lerpStartX) * t
            local cy = lerpStartY + (lerpTargetY - lerpStartY) * t
            SetCameraPosition(cameraEntity, cx, cy)
        end

    -- Lerp zoom
    elseif aType == ACT_LERP_ZOOM then
        zoomTimer = zoomTimer + dt
        local t = zoomTimer / zoomDuration
        if t >= 1.0 then
            t = 1.0
            SetCameraZoom(cameraEntity, zoomTarget)
            NextAction()
        else
            -- Ease out quad
            t = t * (2.0 - t)
            local z = zoomStart + (zoomTarget - zoomStart) * t
            SetCameraZoom(cameraEntity, z)
        end

    -- Wait
    elseif aType == ACT_WAIT then
        waitTimer = waitTimer - dt
        if waitTimer <= 0 then
            NextAction()
        end

    -- Dialogue
    elseif aType == ACT_PLAY_DIALOGUE then
        -- If dialogue failed to open or already closed, skip to next action
        if not dialogueActive then
            NextAction()
            return
        end

        -- Debounce
        if skipFrames > 0 then
            skipFrames = skipFrames - 1
        end

        -- Typewriter tick
        if isTyping then
            revealed = revealed + CHARS_PER_SECOND * dt
            local chars = math.floor(revealed)
            if chars >= #fullText then
                FinishTyping()
            else
                SetBodyText(string.sub(fullText, 1, chars))
            end
        else
            -- Blink indicator
            blinkTimer = blinkTimer + dt
            if blinkTimer >= BLINK_PERIOD then
                blinkTimer = blinkTimer - BLINK_PERIOD
                blinkOn = not blinkOn
                ShowImg(eContinue, blinkOn)
            end
        end

        -- Click or Enter to advance
        local advanceInput = MouseButtonPressed(MOUSE_LEFT)
            or KeyPressed(KEY_ENTER)

        if skipFrames <= 0 and advanceInput then
            if isTyping then
                FinishTyping()
            else
                AdvanceDialogueLine()
                if not dialogueActive then
                    NextAction()
                end
            end
        end
    end
end

-- Shared logic to start a cutscene from a trigger event
local function TryStartCutscene(other, triggerOwner)
    if isPlaying then return end
    if IsCutsceneActive() then return end
    if cooldownTimer > 0 then return end

    if HasPlayerOn(triggerOwner) or HasPlayerOn(other) then
        -- Determine which entity is the trigger owner (has the Cutscene component)
        local cutsceneOwner = triggerOwner
        local cutscene = GetCutsceneFrom(cutsceneOwner)
        if not cutscene then
            -- Try the other entity
            cutsceneOwner = other
            cutscene = GetCutsceneFrom(cutsceneOwner)
        end
        if not cutscene then return end

        if cutscene.playOnce and cutscene.hasPlayed then
            return
        end

        ownerEntity = cutsceneOwner

        -- Spawn dialogue prefab on demand (only one exists at a time)
        if not SpawnDialoguePrefab() then
            Log("[CutsceneTrigger] Failed to spawn dialogue prefab, aborting cutscene")
            return
        end

        -- Load actions
        actions = GetCutsceneActions(ownerEntity)
        if not actions or #actions == 0 then
            Log("[CutsceneTrigger] No cutscene actions defined")
            return
        end

        isPlaying = true
        actionIndex = 1
        cameraEntity = FindCameraEntity()
        SetCutsceneActive(true)

        Log("[CutsceneTrigger] Starting cutscene on entity " .. tostring(EntityID) .. " owner=" .. tostring(ownerEntity) .. " actions=" .. #actions)
        BeginAction()
    end
end

-- Trigger: start cutscene when player enters
function OnTriggerEnter(other, triggerOwner)
    TryStartCutscene(other, triggerOwner)
end

-- Handle ongoing trigger overlap: if the player was already inside this
-- trigger while another cutscene was playing, OnTriggerEnter was consumed.
-- Once that cutscene finishes, this fires and starts the pending cutscene.
function OnTrigger(other, triggerOwner)
    TryStartCutscene(other, triggerOwner)
end

function OnDestroy()
    if isPlaying then
        isPlaying = false
        SetCutsceneActive(false)
    end
    DestroyDialoguePrefab()
end
