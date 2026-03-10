local children
local collider
local missionTextComponent
local missionTextComponent2
local PlayerStatTrackState = require("PlayerStatTrackState")
local triggerCount
local blockerCollider = {}

ExposedVars = {
    missionText = "",
    missionText2 = "",
    missionText3 = "",
    missionText4 = "",
    missionText5 = "",
    missionText6 = "",
    missionText7 = ""
}

function Start()
    if HasCollider() then
        collider = GetCollider()
    end

    triggerCount = 0

    children = GetChildrenList(EntityID)
    blockerCollider[1] = children[6]
    blockerCollider[2] = children[7]
    blockerCollider[3] = children[8]
    blockerCollider[4] = children[9]

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

function OnTriggerEnter()
    -- disable collider so cannot retrigger the same mission
    -- change text for 1st mission
    if triggerCount == 0 then
        RoomTriggerInit()
        SetMissionText(missionText)

    -- change text for 2nd mission
    elseif triggerCount == 1 then
        RoomTriggerInit()
        SetMissionText(missionText4)

    -- change text for 3rd mission
    elseif triggerCount == 2 then
        RoomTriggerInit()
        SetMissionText(missionText7)

    -- change text for 4th mission
    elseif triggerCount == 3 then
        RoomTriggerInit()
        SetMissionText(missionText7)
    end
end

function Update(dt)
    -- update missions to cast each skill once, sequentially for trigger 1,2
    -- eg, cast fire, cast water, cast wind, next level
    -- trigger 3 all enemy die
    -- trigger 4 all enemy die

    -- if player use each elemental skills at least 3x
    if PlayerStatTrackState.GetPassedTrigger() == 1 then
        if PlayerStatTrackState.GetFireAttackCount() >= 3 then
            SetMissionText(missionText2)
        end
        if PlayerStatTrackState.GetWaterAttackCount() >= 3 then
            SetMissionText(missionText3)
        end
        if PlayerStatTrackState.GetWindAttackCount() >= 3 then
            SetMissionText("Move to next room")
            SetActiveEntity(blockerCollider[1], false)
        end
    
    -- if player use each combo skills at least 3x
    elseif PlayerStatTrackState.GetPassedTrigger() == 2 then
        if PlayerStatTrackState.GetPyronadoAttackCount() >= 3 then
            SetMissionText(missionText5)
        end
        if PlayerStatTrackState.GetWhirlpoolAttackCount() >= 3 then
            SetMissionText(missionText6)
        end
        if PlayerStatTrackState.GetSteamburstAttackCount() >= 3 then
            SetMissionText("Move to next room")
            SetActiveEntity(blockerCollider[2], false)
        end

    elseif PlayerStatTrackState.GetPassedTrigger() == 3 then
        if PlayerStatTrackState.GetWaterAttackCount() == 3 then -- SHOULD BE WHEN ENEMY DIES
            SetActiveEntity(blockerCollider[3], false)
        end

    elseif PlayerStatTrackState.GetPassedTrigger() == 4 then
        if PlayerStatTrackState.GetComboAttackCount() == 3 then -- SHOULD BE WHEN ENEMIESSS DIE
            SetActiveEntity(blockerCollider[4], false)
        end
    end
end

function RoomTriggerInit()
    collider.shapes[triggerCount+1].isActive = false
    PlayerStatTrackState.incrPassedTrigger()
    triggerCount = triggerCount + 1
end

function SetMissionText(str)
    missionTextComponent.text = str
    missionTextComponent2.text = str
end