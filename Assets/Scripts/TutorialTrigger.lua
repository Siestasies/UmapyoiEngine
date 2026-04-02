local children
local collider
local missionTextComponent
local missionTextComponent2
local PlayerStatTrackState = require("PlayerStatTrackState")
local triggerCount
local blockerCollider = {}
local rooms_trigger_id = {}
local roomEnemy = {}
local curr_room = 0

-- Root children (1-based):
-- [1]  bg_dup               (UI; child[1]="mission text" -> child[1]="mission text shadow")
-- [2]  Blocker Collider 1   (id 5)
-- [3]  Blocker Collider 2   (id 9)
-- [4]  Blocker Collider 3   (id 13)
-- [5]  Blocker Collider 4   (id 17)
-- [6]  static fire enemy    (id 20) -> room 2 enemy
-- [7]  static water enemy   (id 24) -> room 3 enemy
-- [8]  static wind enemy    (id 28) -> room 1 enemy
-- [9]  trigger room 1       (id 31)
-- [10] trigger room 2       (id 32)
-- [11] trigger room 3       (id 33)
-- [12] trigger room 4       (id 34)

ExposedVars = {
    killWindPrompt  = "Defeat the Wind enemy",
    killFirePrompt  = "Defeat the Fire enemy",
    killWaterPrompt = "Defeat the Water enemy",
    moveToNext      = "Move to the next room",

    -- Marquee settings (tweak these in the editor)
    marqueeWidth    = 24,    -- visible character window width
    marqueeSpeed    = 12.0,  -- characters per second
    marqueePadding  = 6,     -- blank spaces inserted between loops
}

-- -----------------------------------------------------------------------
-- Marquee state
-- -----------------------------------------------------------------------
local marqueeFullStr  = ""   -- the looping string (text + padding)
local marqueeOffset   = 0.0  -- fractional character offset
local marqueeLen      = 0    -- length of marqueeFullStr

local function BuildMarqueeString(str)
    local padding = string.rep(" ", marqueePadding)
    return str .. padding  -- loops back to start seamlessly
end

local function GetMarqueeWindow()
    local idx    = math.floor(marqueeOffset) % marqueeLen
    local result = {}
    for i = 0, marqueeWidth - 1 do
        local ci = (idx + i) % marqueeLen
        result[i + 1] = marqueeFullStr:sub(ci + 1, ci + 1)
    end
    return table.concat(result)
end

-- -----------------------------------------------------------------------
-- Startup
-- -----------------------------------------------------------------------
function Start()
    if HasCollider() then
        collider = GetCollider()
    end

    triggerCount = 0
    children = GetChildrenList(EntityID)

    -- Blocker colliders
    blockerCollider[1] = children[2]
    blockerCollider[2] = children[3]
    blockerCollider[3] = children[4]
    blockerCollider[4] = children[5]

    -- Room enemies
    roomEnemy[1] = children[8]
    roomEnemy[2] = children[6]
    roomEnemy[3] = children[7]

    -- Room entry triggers
    rooms_trigger_id[1] = children[9]
    rooms_trigger_id[2] = children[10]
    rooms_trigger_id[3] = children[11]
    rooms_trigger_id[4] = children[12]

    -- Mission text components
    local bg_dup            = children[1]
    local bg_dup_children   = GetChildrenList(bg_dup)
    local missionTextEntity = bg_dup_children[1]
    local shadowChildren    = GetChildrenList(missionTextEntity)
    local shadowEntity      = shadowChildren[1]

    missionTextComponent  = GetTextFrom(missionTextEntity)
    missionTextComponent2 = GetTextFrom(shadowEntity)

    PlayerStatTrackState.SetPassedTrigger(0)
end

-- -----------------------------------------------------------------------
-- Helpers
-- -----------------------------------------------------------------------
local function IsAnyShapeTriggered(col)
    for i = 1, #col.shapes do
        if col.shapes[i].isTriggered then
            return true
        end
    end
    return false
end

--- Queue a new string into the marquee.
--- Resets the scroll position so the new message starts from the left.
function SetMissionText(str)
    if str == marqueeFullStr:sub(1, #str) and marqueeLen > 0 then
        return  -- already showing this string, don't reset
    end
    marqueeFullStr = BuildMarqueeString(str)
    marqueeLen     = #marqueeFullStr
    marqueeOffset  = 0.0
end

-- -----------------------------------------------------------------------
-- Update
-- -----------------------------------------------------------------------
function Update(dt)
    -- -----------------------------------------------------------------------
    -- Room entry detection (fires only once, strictly in order)
    -- -----------------------------------------------------------------------

    if curr_room < 1 then
        local t = GetColliderFrom(rooms_trigger_id[1])
        if t and IsAnyShapeTriggered(t) then
            curr_room = 1
            RoomTriggerInit()
            SetMissionText(killWindPrompt)
        end
    end

    if curr_room < 2 then
        local t = GetColliderFrom(rooms_trigger_id[2])
        if t and IsAnyShapeTriggered(t) then
            curr_room = 2
            RoomTriggerInit()
            SetMissionText(killFirePrompt)
        end
    end

    if curr_room < 3 then
        local t = GetColliderFrom(rooms_trigger_id[3])
        if t and IsAnyShapeTriggered(t) then
            curr_room = 3
            RoomTriggerInit()
            SetMissionText(killWaterPrompt)
        end
    end

    if curr_room < 4 then
        local t = GetColliderFrom(rooms_trigger_id[4])
        if t and IsAnyShapeTriggered(t) then
            curr_room = 4
            RoomTriggerInit()
            SetActiveEntity(blockerCollider[4], false)
            SetMissionText(moveToNext)
        end
    end

    -- -----------------------------------------------------------------------
    -- Per-room mission logic: unlock blocker when room enemy is destroyed
    -- -----------------------------------------------------------------------

    if curr_room == 1 then
        if not IsEntityValid(roomEnemy[1]) then
            SetActiveEntity(blockerCollider[1], false)
            SetMissionText(moveToNext)
        else
            SetMissionText(killWindPrompt)
        end

    elseif curr_room == 2 then
        if not IsEntityValid(roomEnemy[2]) then
            SetActiveEntity(blockerCollider[2], false)
            SetMissionText(moveToNext)
        else
            SetMissionText(killFirePrompt)
        end

    elseif curr_room == 3 then
        if not IsEntityValid(roomEnemy[3]) then
            SetActiveEntity(blockerCollider[3], false)
            SetMissionText(moveToNext)
        else
            SetMissionText(killWaterPrompt)
        end

    -- Room 4 is fully handled on entry above
    end

    -- -----------------------------------------------------------------------
    -- Advance marquee and write visible window to both text components
    -- -----------------------------------------------------------------------
    if marqueeLen > 0 then
        marqueeOffset = (marqueeOffset + marqueeSpeed * dt) % marqueeLen
        local visible = GetMarqueeWindow()
        missionTextComponent.text  = visible
        missionTextComponent2.text = visible
    end
end

function RoomTriggerInit()
    PlayerStatTrackState.incrPassedTrigger()
    triggerCount = triggerCount + 1
end