--[[
\file   CutsceneTrigger.lua
\par    Project: GAM250
\par    Course: CSD2451
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

-------------------------------------------------------------------------------
-- State machine
--   IDLE    → waiting for player to enter trigger
--   PENDING → player is in trigger but another cutscene is active; poll until free
--   PLAYING → cutscene actions are executing
--   DONE    → cutscene finished while player is still in trigger; blocks re-trigger
-------------------------------------------------------------------------------
local STATE_IDLE    = 0
local STATE_PENDING = 1
local STATE_PLAYING = 2
local STATE_DONE    = 3

local state = STATE_IDLE

-- Prefab entity handles (for dialogue display)
local dialogueRoot = -1
local ePanel       = -1
local eTextField   = -1
local eName        = -1
local eNameField   = -1
local ePortrait    = -1
local eContinue    = -1

-- Cutscene state
local actions       = {}
local actionIndex   = 0
local cameraEntity  = -1
local ownerEntity   = -1
local pendingOther  = -1  -- stored from OnTriggerEnter for deferred start
local pendingOwner  = -1

-- Per-action state: camera lerp
local lerpStartX    = 0
local lerpStartY    = 0
local lerpTargetX   = 0
local lerpTargetY   = 0
local lerpTimer     = 0
local lerpDuration  = 1

-- Per-action state: zoom lerp
local zoomStart     = 10
local zoomTarget    = 10
local zoomTimer     = 0
local zoomDuration  = 1

-- Per-action state: wait
local waitTimer     = 0

-- Current action mode for Update (set by BeginAction, consumed by Update)
local activeMode    = -1

-- Dialogue state
local dialogueActive = false
local dialogueLines  = {}
local lineIndex      = 0
local fullText       = ""
local revealed       = 0.0
local isTyping       = false
local blinkTimer     = 0.0
local blinkOn        = true
local inputCooldown  = 0.0  -- seconds to ignore input after dialogue opens

-------------------------------------------------------------------------------
-- Helpers
-------------------------------------------------------------------------------
local function ShowImg(e, visible)
    if e == -1 or not IsEntityValid(e) then return end
    SetActiveEntity(e, visible)
end

local function ShowTxt(e, visible)
    if e == -1 or not IsEntityValid(e) then return end
    local t = GetTextFrom(e)
    if t then t.visible = visible end
end

local function SetBodyText(str)
    if eTextField == -1 or not IsEntityValid(eTextField) then return end
    local t = GetTextFrom(eTextField)
    if t then t.text = str end
end

local function SetNameText(str)
    if eNameField == -1 or not IsEntityValid(eNameField) then return end
    local t = GetTextFrom(eNameField)
    if t then t.text = str end
end

-------------------------------------------------------------------------------
-- Dialogue helpers
-------------------------------------------------------------------------------
local function ApplyLine(line)
    local hasSpeaker  = line.speaker  and line.speaker  ~= ""
    local hasPortrait = line.portrait and line.portrait ~= ""

    ShowImg(eName,      hasSpeaker)
    ShowImg(eNameField, hasSpeaker)
    ShowTxt(eNameField, hasSpeaker)
    if hasSpeaker then SetNameText(line.speaker) end

    ShowImg(ePortrait, hasPortrait)
    if hasPortrait then
        local img = GetImageFrom(ePortrait)
        if img then
            img.textureName = line.portrait
            img.change = true
        end
    end

    fullText    = line.text or ""
    revealed    = 0.0
    isTyping    = true
    blinkTimer  = 0.0
    blinkOn     = true

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
    if dialogueRoot ~= -1 and IsEntityValid(dialogueRoot) then
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

-------------------------------------------------------------------------------
-- Dialogue prefab management
-------------------------------------------------------------------------------
local function ChildrenValid()
    return ePanel     ~= -1 and IsEntityValid(ePanel)
       and eTextField ~= -1 and IsEntityValid(eTextField)
       and eName      ~= -1 and IsEntityValid(eName)
       and eNameField ~= -1 and IsEntityValid(eNameField)
end

local function ResolveDialogueChildren()
    if dialogueRoot == -1 or not IsEntityValid(dialogueRoot) then return false end

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

    return ePanel ~= -1 and eTextField ~= -1
       and eName ~= -1 and eNameField ~= -1
end

local function OpenDialogue(seqId)
    if dialogueRoot == -1 then
        Log("[CutsceneTrigger] Cannot play dialogue: prefab not spawned")
        dialogueActive = false
        return
    end

    -- Re-resolve children only if cached handles are stale
    if not ChildrenValid() then
        if not ResolveDialogueChildren() then
            Log("[CutsceneTrigger] Cannot play dialogue: children not resolved")
            dialogueActive = false
            return
        end
    end

    local seq = GetDialogueSequence(ownerEntity, seqId)
    if not seq or #seq == 0 then
        Log("[CutsceneTrigger] Dialogue sequence not found: " .. tostring(seqId))
        dialogueActive = false
        return
    end

    dialogueLines  = seq
    lineIndex      = 1
    inputCooldown  = 0.15  -- 150ms grace period to avoid click-through
    dialogueActive = true

    if IsEntityValid(dialogueRoot) then SetActiveEntity(dialogueRoot, true) end
    if ePanel      ~= -1 and IsEntityValid(ePanel)      then SetActiveEntity(ePanel, true) end
    if eTextField   ~= -1 and IsEntityValid(eTextField)  then SetActiveEntity(eTextField, true) end
    if eName        ~= -1 and IsEntityValid(eName)       then SetActiveEntity(eName, true) end
    if eNameField   ~= -1 and IsEntityValid(eNameField)  then SetActiveEntity(eNameField, true) end
    if ePortrait    ~= -1 and IsEntityValid(ePortrait)   then SetActiveEntity(ePortrait, true) end
    if eContinue    ~= -1 and IsEntityValid(eContinue)   then SetActiveEntity(eContinue, true) end

    ApplyLine(dialogueLines[1])
end

local function SpawnDialoguePrefab()
    if dialogueRoot ~= -1 then
        if IsEntityValid(dialogueRoot) then return true end
        -- Entity was destroyed externally; reset handles
        dialogueRoot = -1
        ePanel       = -1
        eTextField   = -1
        eName        = -1
        eNameField   = -1
        ePortrait    = -1
        eContinue    = -1
    end

    dialogueRoot = SpawnPrefab(prefabName, Vec2(0, 0))
    if dialogueRoot == -1 then
        LogError("[CutsceneTrigger] Failed to spawn: " .. tostring(prefabName))
        return false
    end

    ResolveDialogueChildren()
    SetActiveEntity(dialogueRoot, false)
    return true
end

local function DestroyDialoguePrefab()
    if dialogueRoot ~= -1 then
        if IsEntityValid(dialogueRoot) then
            DestroyWithChildren(dialogueRoot)
        end
        dialogueRoot = -1
        ePanel       = -1
        eTextField   = -1
        eName        = -1
        eNameField   = -1
        ePortrait    = -1
        eContinue    = -1
    end
end

-------------------------------------------------------------------------------
-- Cutscene action engine
-------------------------------------------------------------------------------
local NextAction
local BeginAction

local function EndCutscene(reason)
    state       = STATE_DONE
    activeMode  = -1
    actionIndex = 0
    actions     = {}
    SetCutsceneActive(false)
    if ownerEntity ~= -1 and IsEntityValid(ownerEntity) then
        SetCutscenePlayed(ownerEntity, true)
    end
    CloseDialogue()
    DestroyDialoguePrefab()
    Log("[CutsceneTrigger] Cutscene finished (" .. reason .. ") entity=" .. tostring(EntityID))
end

NextAction = function()
    actionIndex = actionIndex + 1
    if actionIndex > #actions then
        EndCutscene("complete")
        return
    end
    BeginAction()
end

BeginAction = function()
    local action = actions[actionIndex]
    if not action then
        EndCutscene("nil action")
        return
    end

    local aType = action.type
    activeMode = aType

    if aType == ACT_SET_CAMERA then
        SetCameraFollow(cameraEntity, false)
        SetCameraPosition(cameraEntity, action.targetX, action.targetY)
        NextAction()

    elseif aType == ACT_LERP_CAMERA then
        SetCameraFollow(cameraEntity, false)
        lerpStartX, lerpStartY = GetTransformPosition(cameraEntity)
        lerpTargetX  = action.targetX
        lerpTargetY  = action.targetY
        lerpDuration = action.duration or 1
        lerpTimer    = 0
        if lerpDuration <= 0 then
            SetCameraPosition(cameraEntity, lerpTargetX, lerpTargetY)
            NextAction()
        end

    elseif aType == ACT_PLAY_DIALOGUE then
        OpenDialogue(action.dialogueSequenceId)
        if not dialogueActive then
            NextAction()
        end

    elseif aType == ACT_WAIT then
        waitTimer = action.duration or 0
        if waitTimer <= 0 then
            NextAction()
        end

    elseif aType == ACT_RETURN_CAMERA then
        SetCameraZoom(cameraEntity, 10)
        SetCameraFollow(cameraEntity, true)
        NextAction()

    elseif aType == ACT_SHAKE_CAMERA then
        CameraShake(cameraEntity, action.shakeIntensity, action.duration)
        NextAction()

    elseif aType == ACT_LERP_ZOOM then
        zoomStart    = GetCameraZoom(cameraEntity)
        zoomTarget   = action.targetZoom
        zoomDuration = action.duration or 1
        zoomTimer    = 0
        if zoomDuration <= 0 then
            SetCameraZoom(cameraEntity, zoomTarget)
            NextAction()
        end

    else
        Log("[CutsceneTrigger] Unknown action type " .. tostring(aType) .. ", skipping")
        NextAction()
    end
end

-------------------------------------------------------------------------------
-- Try to start a cutscene (shared by trigger entry and pending-poll)
-------------------------------------------------------------------------------
local function TryStartCutscene(other, triggerOwner)
    -- Require player involvement
    if not (HasPlayerOn(triggerOwner) or HasPlayerOn(other)) then
        return false
    end

    -- Find the entity that owns the Cutscene component
    local cutsceneOwner = triggerOwner
    local cutscene = GetCutsceneFrom(cutsceneOwner)
    if not cutscene then
        cutsceneOwner = other
        cutscene = GetCutsceneFrom(cutsceneOwner)
    end
    if not cutscene then return false end

    if cutscene.playOnce and cutscene.hasPlayed then
        return false
    end

    ownerEntity = cutsceneOwner

    if not SpawnDialoguePrefab() then
        Log("[CutsceneTrigger] Failed to spawn dialogue prefab, aborting")
        return false
    end

    actions = GetCutsceneActions(ownerEntity)
    if not actions or #actions == 0 then
        Log("[CutsceneTrigger] No cutscene actions defined")
        return false
    end

    state       = STATE_PLAYING
    activeMode  = -1
    actionIndex = 1
    cameraEntity = FindCameraEntity()
    SetCutsceneActive(true)

    Log("[CutsceneTrigger] Starting cutscene entity=" .. tostring(EntityID)
        .. " owner=" .. tostring(ownerEntity)
        .. " actions=" .. #actions)
    BeginAction()
    return true
end

-------------------------------------------------------------------------------
-- Per-frame action update (only called when state == STATE_PLAYING)
-------------------------------------------------------------------------------
local function UpdateAction(dt)
    -- Lerp camera position
    if activeMode == ACT_LERP_CAMERA then
        lerpTimer = lerpTimer + dt
        local t = lerpTimer / lerpDuration
        if t >= 1.0 then
            SetCameraPosition(cameraEntity, lerpTargetX, lerpTargetY)
            NextAction()
        else
            t = t * (2.0 - t)  -- ease-out quad
            local cx = lerpStartX + (lerpTargetX - lerpStartX) * t
            local cy = lerpStartY + (lerpTargetY - lerpStartY) * t
            SetCameraPosition(cameraEntity, cx, cy)
        end

    -- Lerp camera zoom
    elseif activeMode == ACT_LERP_ZOOM then
        zoomTimer = zoomTimer + dt
        local t = zoomTimer / zoomDuration
        if t >= 1.0 then
            SetCameraZoom(cameraEntity, zoomTarget)
            NextAction()
        else
            t = t * (2.0 - t)
            local z = zoomStart + (zoomTarget - zoomStart) * t
            SetCameraZoom(cameraEntity, z)
        end

    -- Wait
    elseif activeMode == ACT_WAIT then
        waitTimer = waitTimer - dt
        if waitTimer <= 0 then
            NextAction()
        end

    -- Dialogue
    elseif activeMode == ACT_PLAY_DIALOGUE then
        if not dialogueActive then
            NextAction()
            return
        end

        -- Input cooldown (time-based, not frame-based)
        if inputCooldown > 0 then
            inputCooldown = inputCooldown - dt
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
            -- Blink continue indicator
            blinkTimer = blinkTimer + dt
            if blinkTimer >= BLINK_PERIOD then
                blinkTimer = blinkTimer - BLINK_PERIOD
                blinkOn = not blinkOn
                ShowImg(eContinue, blinkOn)
            end
        end

        -- Advance on click / enter
        if inputCooldown <= 0 then
            local advance = MouseButtonPressed(MOUSE_LEFT) or KeyPressed(KEY_ENTER)
            if advance then
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
    -- Instant actions (SET_CAMERA, RETURN_CAMERA, SHAKE_CAMERA) never reach
    -- Update because BeginAction calls NextAction immediately for those.
end

-------------------------------------------------------------------------------
-- Lifecycle callbacks
-------------------------------------------------------------------------------
function Start()
    dialogueRoot = -1
    ePanel       = -1
    eTextField   = -1
    eName        = -1
    eNameField   = -1
    ePortrait    = -1
    eContinue    = -1

    state          = STATE_IDLE
    activeMode     = -1
    actions        = {}
    actionIndex    = 0
    ownerEntity    = -1
    pendingOther   = -1
    pendingOwner   = -1
    dialogueActive = false
    dialogueLines  = {}
    lineIndex      = 0

    SetCutsceneActive(false)

    cameraEntity = FindCameraEntity()

    -- Pre-spawn dialogue prefab so textures/fonts have time to load
    SpawnDialoguePrefab()

    Log("[CutsceneTrigger] Ready. Entity=" .. tostring(EntityID) .. " Camera=" .. tostring(cameraEntity))
end

function Update(dt)
    cameraEntity = FindCameraEntity()

    if state == STATE_IDLE then
        -- Nothing to do
        return

    elseif state == STATE_PENDING then
        -- Another cutscene was active when the player entered this trigger.
        -- Poll until the global flag clears, then start ours.
        if not IsCutsceneActive() then
            if not IsEntityValid(pendingOther) or not IsEntityValid(pendingOwner) then
                -- Stored entities were destroyed while waiting; abandon
                state = STATE_IDLE
                pendingOther = -1
                pendingOwner = -1
            elseif not TryStartCutscene(pendingOther, pendingOwner) then
                -- Can't start (playOnce already played, etc.) — go idle
                state = STATE_DONE
            end
        end
        return

    elseif state == STATE_PLAYING then
        UpdateAction(dt)
        return

    elseif state == STATE_DONE then
        -- Cutscene finished; restore camera follow once, then go idle
        if not IsCutsceneActive() and cameraEntity ~= -1 and IsEntityValid(cameraEntity) then
            SetCameraFollow(cameraEntity, true)
            state = STATE_IDLE
        end
        return
    end
end

-------------------------------------------------------------------------------
-- Trigger callbacks
-------------------------------------------------------------------------------
function OnTriggerEnter(other, triggerOwner)
    -- New overlap always resets DONE so re-entering the trigger works
    if state == STATE_DONE then
        state = STATE_IDLE
    end

    if state ~= STATE_IDLE then return end

    if IsCutsceneActive() then
        -- Another cutscene is playing; remember the args and wait
        pendingOther = other
        pendingOwner = triggerOwner
        state = STATE_PENDING
        return
    end

    if not TryStartCutscene(other, triggerOwner) then
        -- Couldn't start (not player, playOnce, no actions, etc.)
        return
    end
end

-- OnTrigger is intentionally NOT used. The PENDING state in Update handles
-- the case where the player was already inside this trigger while another
-- cutscene was playing. This avoids the re-trigger loop that OnTrigger caused.

-------------------------------------------------------------------------------
-- Cleanup
-------------------------------------------------------------------------------
function Shutdown()
    if state == STATE_PLAYING then
        SetCutsceneActive(false)
    end
    CloseDialogue()
    DestroyDialoguePrefab()

    state       = STATE_IDLE
    activeMode  = -1
    actions     = {}
    actionIndex = 0
    ownerEntity = -1
end

function OnDestroy()
    if state == STATE_PLAYING then
        SetCutsceneActive(false)
    end
    DestroyDialoguePrefab()
end
