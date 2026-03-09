local children
local collider
local missionTextComponent
local missionTextComponent2
local PlayerStatTrackState = require("PlayerStatTrackState")
local triggerCount
local blockerCollider

ExposedVars = {
    missionText = "",
    missionText2 = "",
    missionText3 = "",
    missionText4 = ""
}

function Start()
    if HasCollider() then
        collider = GetCollider()
    end

    triggerCount = 0

    children = GetChildrenList(EntityID)
    blockerCollider = children[5]

    missionTextComponent = GetTextFrom(children[2])
    missionTextComponent2 = GetTextFrom(children[3])

    PlayerStatTrackState.SetFireAttackCount(0)
    PlayerStatTrackState.SetWaterAttackCount(0)
    PlayerStatTrackState.SetWindAttackCount(0)
    PlayerStatTrackState.SetComboAttackCount(0)
    PlayerStatTrackState.SetPassedTrigger(0)
end

function OnTriggerEnter()
    -- disable collider so cannot retrigger the same mission
    -- change text for 1st mission, water attack
    if triggerCount == 0 then
        collider.shapes[triggerCount].isActive = false
        PlayerStatTrackState.incrPassedTrigger()
        missionTextComponent.text = missionText
        missionTextComponent2.text = missionText
        triggerCount = triggerCount + 1

    -- change text for 2nd mission, wind attack
    elseif triggerCount == 1 then
        collider.shapes[triggerCount].isActive = false
        PlayerStatTrackState.incrPassedTrigger()
        missionTextComponent.text = missionText2
        missionTextComponent2.text = missionText2
        triggerCount = triggerCount + 1

    -- change text for 3rd mission, fire attack
    elseif triggerCount == 2 then
        collider.shapes[triggerCount].isActive = false
        PlayerStatTrackState.incrPassedTrigger()
        missionTextComponent.text = missionText3
        missionTextComponent2.text = missionText3
        triggerCount = triggerCount + 1

    -- change text for 4th mission, combo attack
    elseif triggerCount == 3 then
        collider.shapes[triggerCount].isActive = false
        PlayerStatTrackState.incrPassedTrigger()
        missionTextComponent.text = missionText4
        missionTextComponent2.text = missionText4
        triggerCount = triggerCount + 1
    end
end

function Update(dt)
    --Log("XXXXXXXXXXXXXXXXXXXXXXXXXXX  current trigger count " ..  PlayerStatTrackState.GetPassedTrigger())

    -- if player use fire attack 3 times, disable blocking collider
    if PlayerStatTrackState.GetPassedTrigger() == 1 then
        if PlayerStatTrackState.GetFireAttackCount() == 3 then
            -- play breaking animation clip?
            --blockerCollider.shapes[triggerCount-1].isActive = false
        end
    
    elseif PlayerStatTrackState.GetPassedTrigger() == 2 then
        if PlayerStatTrackState.GetWindAttackCount() == 3 then
            --blockerCollider.shapes[triggerCount-1].isActive = false
            SetActiveEntity(blockerCollider, true)
        end

    elseif PlayerStatTrackState.GetPassedTrigger() == 3 then
        if PlayerStatTrackState.GetWaterAttackCount() == 3 then
            --blockerCollider.shapes[triggerCount-1].isActive = false
            SetActiveEntity(blockerCollider, false)
        end

    elseif PlayerStatTrackState.GetPassedTrigger() == 4 then
        if PlayerStatTrackState.GetComboAttackCount() == 3 then
            --blockerCollider.shapes[triggerCount-1].isActive = false
            SetActiveEntity(blockerCollider, true)
        end
    end
end