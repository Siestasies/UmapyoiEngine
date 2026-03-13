local children
local collider
local missionTextComponent
local missionTextComponent2
local PlayerStatTrackState = require("PlayerStatTrackState")
local triggerCount
local blockerCollider = {}
local tutorialEnemies = {}
local rooms_trigger_id = {}
local curr_room = 0

ExposedVars = {
    missionText = "",
    missionText2 = "",
    missionText3 = "",
    missionText4 = "",
    missionText5 = "",
    missionText6 = "",
    missionText7 = "",
    moveToNext = "",
}

function Start()
    if HasCollider() then
        collider = GetCollider()
    end

    triggerCount = 0

    children = GetChildrenList(EntityID)
    -- stage blockers ref
    blockerCollider[1] = children[6]
    blockerCollider[2] = children[7]
    blockerCollider[3] = children[8]
    blockerCollider[4] = children[9]

    -- static enemy refs
    tutorialEnemies[1] = children[10]
    tutorialEnemies[2] = children[11]

    rooms_trigger_id[1] = children[12]
    rooms_trigger_id[2] = children[13]
    rooms_trigger_id[3] = children[14]
    rooms_trigger_id[4] = children[15]

    -- mission text refs
    missionTextComponent = GetTextFrom(children[3])
    missionTextComponent2 = GetTextFrom(children[4])

    PlayerStatTrackState.SetFireAttackCount(0)
    PlayerStatTrackState.SetWaterAttackCount(0)
    PlayerStatTrackState.SetWindAttackCount(0)
    PlayerStatTrackState.SetPyronadoAttackCount(0)
    PlayerStatTrackState.SetWhirlpoolAttackCount(0)
    PlayerStatTrackState.SetSteamburstAttackCount(0)
    PlayerStatTrackState.SetPassedTrigger(0)
end

--- Check if any shape on a collider is currently triggered
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
    -- Room 1: detect entry (only once)
    if curr_room < 1 then
        local room1_trigger = GetColliderFrom(rooms_trigger_id[1])
        if room1_trigger and IsAnyShapeTriggered(room1_trigger) then
            curr_room = 1
            RoomTriggerInit()
            SetMissionText(missionText)
            SpawnPrefab("Tutorial Popup2.prefab", Vec2(0,0))
        end
    end

    -- Room 2: detect entry (only once)
    if curr_room < 2 then
        local room2_trigger = GetColliderFrom(rooms_trigger_id[2])
        if room2_trigger and IsAnyShapeTriggered(room2_trigger) then
            curr_room = 2
            RoomTriggerInit()
            SetMissionText(missionText4)
            SpawnPrefab("Tutorial Popup3.prefab", Vec2(0,0))
        end
    end

    -- Room 3: detect entry (only once)
    if curr_room < 3 then
        local room3_trigger = GetColliderFrom(rooms_trigger_id[3])
        if room3_trigger and IsAnyShapeTriggered(room3_trigger) then
            curr_room = 3
            RoomTriggerInit()
            SetMissionText(missionText7)
        end
    end

    -- Room 4: detect entry (only once)
    if curr_room < 4 then
        local room4_trigger = GetColliderFrom(rooms_trigger_id[4])
        if room4_trigger and IsAnyShapeTriggered(room4_trigger) then
            curr_room = 4
            RoomTriggerInit()
            SetMissionText(missionText7)
        end
    end

    -- Room 1 mission: use each basic element 3x
    if curr_room == 1 then
        local fire = PlayerStatTrackState.GetFireAttackCount()
        local water = PlayerStatTrackState.GetWaterAttackCount()
        local wind = PlayerStatTrackState.GetWindAttackCount()

        if wind >= 3 and water >= 3 and fire >= 3 then
            SetMissionText(moveToNext)
            SetActiveEntity(blockerCollider[1], false)
            DestroyWithChildren(tutorialEnemies[1])
        elseif water >= 3 and fire >= 3 then
            SetMissionText(missionText3 .. (3-wind) .. " times")
        elseif fire >= 3 then
            SetMissionText(missionText2 .. (3-water) .. " times")
        else
            SetMissionText(missionText .. (3-fire) .. " times")
        end

    -- Room 2 mission: use each combo element 3x
    elseif curr_room == 2 then
        local pyro = PlayerStatTrackState.GetPyronadoAttackCount()
        local whirl = PlayerStatTrackState.GetWhirlpoolAttackCount()
        local steam = PlayerStatTrackState.GetSteamburstAttackCount()

        if steam >= 3 and whirl >= 3 and pyro >= 3 then
            SetMissionText(moveToNext)
            SetActiveEntity(blockerCollider[2], false)
            DestroyWithChildren(tutorialEnemies[2])
        elseif whirl >= 3 and pyro >= 3 then
            SetMissionText(missionText6 .. (3-steam) .. " times")
        elseif pyro >= 3 then
            SetMissionText(missionText5 .. (3-whirl) .. " times")
        else
            SetMissionText(missionText4 .. (3-pyro) .. " times")
        end

    -- Room 3: kill enemies down to 2
    elseif curr_room == 3 then
        local enemyCount = CountEntitiesWithComponent("Enemy")
        if enemyCount == 2 then
            SetActiveEntity(blockerCollider[3], false)
            SetMissionText(moveToNext)
        end

    -- Room 4: kill all remaining enemies
    elseif curr_room == 4 then
        local enemyCount = CountEntitiesWithComponent("Enemy")
        if enemyCount == 0 then
            SetActiveEntity(blockerCollider[4], false)
            SetMissionText("Touch the Kappa Shrine")
        end
    end
end

function RoomTriggerInit()
    PlayerStatTrackState.incrPassedTrigger()
    triggerCount = triggerCount + 1
end

function SetMissionText(str)
    missionTextComponent.text = str
    missionTextComponent2.text = str
end
