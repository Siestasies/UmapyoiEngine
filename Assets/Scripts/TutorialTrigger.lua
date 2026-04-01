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
}

function Start()
    if HasCollider() then
        collider = GetCollider()
    end

    triggerCount = 0
    children = GetChildrenList(EntityID)

    -- Blocker colliders (correctly mapped from prefab names)
    blockerCollider[1] = children[2]   -- Blocker Collider 1 (id 5)
    blockerCollider[2] = children[3]   -- Blocker Collider 2 (id 9)
    blockerCollider[3] = children[4]   -- Blocker Collider 3 (id 13)
    blockerCollider[4] = children[5]   -- Blocker Collider 4 (id 17)

    -- Room enemies
    roomEnemy[1] = children[8]   -- wind enemy  -> room 1
    roomEnemy[2] = children[6]   -- fire enemy  -> room 2
    roomEnemy[3] = children[7]   -- water enemy -> room 3

    -- Room entry triggers
    rooms_trigger_id[1] = children[9]    -- "trigger room 1"
    rooms_trigger_id[2] = children[10]   -- "trigger room 2"
    rooms_trigger_id[3] = children[11]   -- "trigger room 3"
    rooms_trigger_id[4] = children[12]   -- "trigger room 4"

    -- Mission text: bg_dup -> "mission text" -> "mission text shadow"
    local bg_dup            = children[1]
    local bg_dup_children   = GetChildrenList(bg_dup)
    local missionTextEntity = bg_dup_children[1]
    local shadowChildren    = GetChildrenList(missionTextEntity)
    local shadowEntity      = shadowChildren[1]

    missionTextComponent  = GetTextFrom(missionTextEntity)
    missionTextComponent2 = GetTextFrom(shadowEntity)

    PlayerStatTrackState.SetPassedTrigger(0)
end

--- Returns true if any shape on the collider is currently triggered
---@param col Collider
---@return boolean
local function IsAnyShapeTriggered(col)
    for i = 1, #col.shapes do
        if col.shapes[i].isTriggered then
            return true
        end
    end
    return false
end

function Update(dt)
    -- -----------------------------------------------------------------------
    -- Room entry detection (each fires only once, strictly in order)
    -- -----------------------------------------------------------------------

    if curr_room < 1 then
        local t = GetColliderFrom(rooms_trigger_id[1])
        if t and IsAnyShapeTriggered(t) then
            curr_room = 1
            RoomTriggerInit()
            SetMissionText(killWindPrompt)
            local prefab = SpawnPrefab("Tutorial Popup2.prefab", Vec2(10000, 10000))
            SetParent(prefab, EntityID)
            GetTransformFrom(prefab).position = Vec2(0, 0)
        end
    end

    if curr_room < 2 then
        local t = GetColliderFrom(rooms_trigger_id[2])
        if t and IsAnyShapeTriggered(t) then
            curr_room = 2
            RoomTriggerInit()
            SetMissionText(killFirePrompt)
            local prefab = SpawnPrefab("Tutorial Popup3.prefab", Vec2(10000, 10000))
            SetParent(prefab, EntityID)
            GetTransformFrom(prefab).position = Vec2(0, 0)
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
            -- Room 4: buff room — unlock immediately on entry
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
end

function RoomTriggerInit()
    PlayerStatTrackState.incrPassedTrigger()
    triggerCount = triggerCount + 1
end

function SetMissionText(str)
    missionTextComponent.text  = str
    missionTextComponent2.text = str
end