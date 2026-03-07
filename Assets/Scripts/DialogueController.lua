--[[
    DialogueController.lua
    Attach to: "Dialogue" (prefab root, id=0).

    Hierarchy (from Dialogue.prefab):
      Dialogue  [EntityID]          – Canvas + DialogueData
        [0] Panel                   – Image (background panel)
              [0] TextField         – Text  (body text) ** child of Panel, NOT root **
        [1] Name                    – Image (name box)
              [0] NameField         – Text  (speaker name)
        [2] Portrait                – Image
        [3] Continue                – Image (blink indicator)

    Public API (called from DialogueTrigger or any other script):
        DialogueOpen(sequenceId)
        DialogueClose()
        DialogueIsOpen() → bool
--]]

-- ─── Config ───────────────────────────────────────────────────────────────────
local CHARS_PER_SECOND = 40
local BLINK_PERIOD     = 0.5   -- seconds per half-cycle

-- ─── State ────────────────────────────────────────────────────────────────────
local lines      = {}    -- array of {speaker, text, portrait} for active sequence
local lineIndex  = 0     -- 1-based; 0 = closed/idle
local fullText   = ""    -- complete text of the current line
local revealed   = 0.0   -- typewriter accumulator (fractional characters)
local isTyping   = false
local blinkTimer = 0.0
local blinkOn    = true
local skipFrames = 0     -- input debounce after opening

-- ─── Entity handle cache (resolved once in Start) ─────────────────────────────
local ePanel     = -1
local eTextField = -1   -- child[0] of Panel
local eName      = -1
local eNameField = -1   -- child[0] of Name
local ePortrait  = -1
local eContinue  = -1

-- ─── Helpers ──────────────────────────────────────────────────────────────────

local function SetBodyText(str)
    local t = GetTextFrom(eTextField)
    if t then t.text = str end
end

local function SetNameText(str)
    local t = GetTextFrom(eNameField)
    if t then t.text = str end
end

local function ShowImg(e, visible)
    if e == -1 then return end
    local img = GetImageFrom(e)
    if img then img.visible = visible end
end

local function ShowTxt(e, visible)
    if e == -1 then return end
    local t = GetTextFrom(e)
    if t then t.visible = visible end
end

-- ─── Line logic ───────────────────────────────────────────────────────────────

local function ApplyLine(line)
    -- Speaker name box
    local hasSpeaker = line.speaker and line.speaker ~= ""
    ShowImg(eName,      hasSpeaker)
    ShowTxt(eNameField, hasSpeaker)
    if hasSpeaker then SetNameText(line.speaker) end

    -- Portrait
    local hasPortrait = line.portrait and line.portrait ~= ""
    ShowImg(ePortrait, hasPortrait)
    if hasPortrait then
        local img = GetImageFrom(ePortrait)
        if img then img.textureName = line.portrait end
    end

    -- Start typewriter for body text
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

local function Advance()
    lineIndex = lineIndex + 1
    if lineIndex > #lines then
        DialogueClose()
    else
        ApplyLine(lines[lineIndex])
    end
end

-- ─── Public API ───────────────────────────────────────────────────────────────

function DialogueOpen(sequenceId)
    local seq = GetDialogueSequence(EntityID, sequenceId)
    if not seq or #seq == 0 then
        Log("[DialogueController] Sequence not found: " .. tostring(sequenceId))
        return
    end

    lines      = seq
    lineIndex  = 1
    skipFrames = 2              -- swallow the click that triggered us

    SetActiveEntity(EntityID, true)
    ApplyLine(lines[1])
    Log("[DialogueController] Opened '" .. sequenceId .. "'")
end

function DialogueClose()
    lineIndex = 0
    lines     = {}
    isTyping  = false
    SetActiveEntity(EntityID, false)
    Log("[DialogueController] Closed")
end

function DialogueIsOpen()
    return lineIndex > 0
end

-- ─── Lifecycle ────────────────────────────────────────────────────────────────

function Start()
    -- Walk the hierarchy exactly as defined in Dialogue.prefab.
    ePanel    = GetChildren(EntityID, 0)   -- "Panel"     child 0 of root
    eName     = GetChildren(EntityID, 1)   -- "Name"      child 1 of root
    ePortrait = GetChildren(EntityID, 2)   -- "Portrait"  child 2 of root
    eContinue = GetChildren(EntityID, 3)   -- "Continue"  child 3 of root

    -- TextField is under Panel, not directly under root.
    if ePanel ~= -1 then
        eTextField = GetChildren(ePanel, 0)
    end

    -- NameField is under Name.
    if eName ~= -1 then
        eNameField = GetChildren(eName, 0)
    end

    -- Hide the whole dialogue canvas until DialogueOpen() is called.
    SetActiveEntity(EntityID, false)

    Log("[DialogueController] Start complete."
        .. " Panel="     .. tostring(ePanel)
        .. " TextField=" .. tostring(eTextField)
        .. " Name="      .. tostring(eName)
        .. " NameField=" .. tostring(eNameField)
        .. " Portrait="  .. tostring(ePortrait)
        .. " Continue="  .. tostring(eContinue))
end

function Update(dt)
    if lineIndex == 0 then return end

    -- Burn debounce frames (typewriter still ticks, input suppressed).
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

    -- Continue indicator blink
    else
        blinkTimer = blinkTimer + dt
        if blinkTimer >= BLINK_PERIOD then
            blinkTimer = blinkTimer - BLINK_PERIOD
            blinkOn    = not blinkOn
            ShowImg(eContinue, blinkOn)
        end
    end

    -- Left-click advances the dialogue (single-frame press only).
    if skipFrames <= 0 and MouseButtonPressed(MOUSE_LEFT) then
        if isTyping then
            FinishTyping()   -- first click: skip to end of line
        else
            Advance()        -- second click: next line or close
        end
    end
end