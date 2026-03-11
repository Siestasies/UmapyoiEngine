--[[
    DialogueTrigger.lua
    Attach to: any NPC or interactable entity.

    This script is fully self-contained. It spawns the dialogue prefab, resolves
    all child entity handles, and runs the full dialogue loop (typewriter, blink,
    click-to-advance) itself.  No cross-script function calls are needed.

    Prefab hierarchy expected (Dialogue.prefab):
      Dialogue  [dialogueRoot]           Canvas + Dialogue component
        [0] Panel                        Image (panel background)
              [0] TextField              Text  (body text) -- child of Panel
        [1] Name                         Image (name box)
              [0] NameField              Text  (speaker name)
        [2] Portrait                     Image
        [3] Continue                     Image (blink indicator)

    ExposedVars (edit per NPC in the inspector):
        sequenceId  – which sequence to play from the Dialogue component
        prefabName  – prefab file name (include .prefab extension)
        interactKey – integer key code; default KEY_E
--]]

-- ─── ExposedVars ─────────────────────────────────────────────────────────────
-- Flat key=value table. Types are inferred: string→T_STRING, number→T_FLOAT.
-- interactKey intentionally left out of ExposedVars because sol2 infers all
-- numbers as T_FLOAT, and KeyPressed expects an int. We keep it as a plain
-- local instead and let the designer change it in source if needed.
ExposedVars = {
    sequenceId = "Intro",
    prefabName = "Dialogue.prefab",
}

sequenceId  = "Intro"
prefabName  = "Dialogue.prefab"

local INTERACT_KEY = KEY_E    -- change here if needed; never goes through ExposedVars

-- ─── Dialogue config ─────────────────────────────────────────────────────────
local CHARS_PER_SECOND = 40
local BLINK_PERIOD     = 0.5

-- ─── Prefab entity handles ───────────────────────────────────────────────────
local dialogueRoot = -1
local ePanel       = -1
local eTextField   = -1   -- child[0] of Panel
local eName        = -1
local eNameField   = -1   -- child[0] of Name
local ePortrait    = -1
local eContinue    = -1

-- ─── Runtime dialogue state ──────────────────────────────────────────────────
local lines      = {}
local lineIndex  = 0     -- 0 = closed
local fullText   = ""
local revealed   = 0.0
local isTyping   = false
local blinkTimer = 0.0
local blinkOn    = true
local skipFrames = 0

-- ─── Helpers ─────────────────────────────────────────────────────────────────

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

-- ─── Dialogue control ────────────────────────────────────────────────────────

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

local function AdvanceLine()
    lineIndex = lineIndex + 1
    if lineIndex > #lines then
        -- End of sequence: hide the dialogue box
        lineIndex = 0
        lines     = {}
        isTyping  = false
        SetActiveEntity(dialogueRoot, false)
        Log("[DialogueTrigger] Dialogue closed")
    else
        ApplyLine(lines[lineIndex])
    end
end

local function OpenDialogue()
    -- GetDialogueSequence reads from the Dialogue component on dialogueRoot
    local seq = GetDialogueSequence(dialogueRoot, sequenceId)
    if not seq or #seq == 0 then
        Log("[DialogueTrigger] Sequence not found: " .. tostring(sequenceId))
        return
    end

    lines      = seq
    lineIndex  = 1
    skipFrames = 2

    SetActiveEntity(dialogueRoot, true)
    ApplyLine(lines[1])
    Log("[DialogueTrigger] Opened sequence '" .. sequenceId .. "'")
end

-- ─── Lifecycle ───────────────────────────────────────────────────────────────

function Start()
    -- Spawn the prefab and resolve all child handles once.
    dialogueRoot = SpawnPrefab(prefabName, Vec2(0, 0))

    if dialogueRoot == -1 then
        LogError("[DialogueTrigger] Failed to spawn: " .. tostring(prefabName))
        return
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

    -- Hide until triggered
    SetActiveEntity(dialogueRoot, false)

    Log("[DialogueTrigger] Ready."
        .. " root="      .. tostring(dialogueRoot)
        .. " Panel="     .. tostring(ePanel)
        .. " TextField=" .. tostring(eTextField)
        .. " Name="      .. tostring(eName)
        .. " NameField=" .. tostring(eNameField)
        .. " Portrait="  .. tostring(ePortrait)
        .. " Continue="  .. tostring(eContinue))
end

function Update(dt)
    if dialogueRoot == -1 then return end

    -- ── Open on interact key (only when closed) ───────────────────────────────
    if lineIndex == 0 then
        if KeyPressed(INTERACT_KEY) then
            OpenDialogue()
        end
        return
    end

    -- ── Debounce ─────────────────────────────────────────────────────────────
    if skipFrames > 0 then
        skipFrames = skipFrames - 1
    end

    -- ── Typewriter tick ───────────────────────────────────────────────────────
    if isTyping then
        revealed = revealed + CHARS_PER_SECOND * dt
        local chars = math.floor(revealed)
        if chars >= #fullText then
            FinishTyping()
        else
            SetBodyText(string.sub(fullText, 1, chars))
        end

    -- ── Continue indicator blink ──────────────────────────────────────────────
    else
        blinkTimer = blinkTimer + dt
        if blinkTimer >= BLINK_PERIOD then
            blinkTimer = blinkTimer - BLINK_PERIOD
            blinkOn    = not blinkOn
            ShowImg(eContinue, blinkOn)
        end
    end

    -- ── Click to advance ──────────────────────────────────────────────────────
    if skipFrames <= 0 and MouseButtonPressed(MOUSE_LEFT) then
        if isTyping then
            FinishTyping()
        else
            AdvanceLine()
        end
    end
end