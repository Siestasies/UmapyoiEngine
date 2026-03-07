--[[
    DialogueTrigger.lua
    Attach to: any NPC or interactable entity in the scene.

    This script owns the dialogue prefab instance.  When the player presses the
    interact key (KEY_E) while the dialogue is closed it calls DialogueOpen() on
    the controller, kicking off the configured sequence.  When the dialogue is
    open the same key is blocked so normal gameplay input does not bleed through.

    Inspector-exposed variables (edit in the editor per NPC):
        sequenceId   – which sequence to play from the prefab's DialogueData
        prefabName   – prefab file to spawn (must include .prefab extension)
        interactKey  – key constant used to open the dialogue
--]]

ExposedVars = {
    { name = "sequenceId",  type = "string", value = "Intro"             },
    { name = "prefabName",  type = "string", value = "Dialogue.prefab"   },
    { name = "interactKey", type = "int",    value = KEY_E               },
}

-- ─── Exposed variables (defaults; overwritten by ExposedVars sync) ────────────
sequenceId  = "Intro"
prefabName  = "Dialogue.prefab"
interactKey = KEY_E

-- ─── Private state ────────────────────────────────────────────────────────────
local dialogueRoot = -1   -- entity id of the spawned prefab root

-- ─── Helpers ──────────────────────────────────────────────────────────────────

-- Calls a named function in the DialogueController script environment.
-- Because scripts share the global Lua state, functions declared as globals
-- in DialogueController.lua (DialogueOpen, DialogueClose, DialogueIsOpen)
-- are accessible directly.

local function IsDialogueOpen()
    if dialogueRoot == -1 then return false end
    -- DialogueIsOpen is a global set by DialogueController.lua
    return DialogueIsOpen ~= nil and DialogueIsOpen()
end

-- ─── Lifecycle ────────────────────────────────────────────────────────────────

function Start()
    -- Spawn the dialogue prefab once and keep the root entity cached.
    -- It starts hidden (DialogueController.Start() calls SetActiveEntity false).
    dialogueRoot = SpawnPrefab(prefabName, Vec2(0, 0))

    if dialogueRoot == -1 then
        LogError("[DialogueTrigger] Failed to spawn prefab: " .. tostring(prefabName))
    else
        Log("[DialogueTrigger] Spawned '" .. prefabName
            .. "' → root=" .. tostring(dialogueRoot))
    end
end

function Update(dt)
    if dialogueRoot == -1 then return end

    if KeyPressed(interactKey) then
        if not IsDialogueOpen() then
            -- Open: call DialogueOpen on the controller with our sequence id.
            -- DialogueOpen is a global function defined in DialogueController.lua.
            if DialogueOpen then
                DialogueOpen(sequenceId)
            else
                LogError("[DialogueTrigger] DialogueOpen not found – is DialogueController.lua loaded?")
            end
        end
        -- If dialogue is already open, KEY_E does nothing here.
        -- DialogueController handles click-to-advance independently.
    end
end