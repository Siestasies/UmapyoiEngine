local children
local blockerCollider
local missionTextComponent
local PlayerStatTrackState = require("PlayerStatTrackState")

ExposedVars = {
    missionText = "",
}

function Start()
    children = GetChildrenList(EntityID)
    blockerCollider = children[4]
    missionTextComponent = GetTextFrom(children[2])

    PlayerStatTrackState.SetFireAttackCount(0)
    PlayerStatTrackState.SetWaterAttackCount(0)
    PlayerStatTrackState.SetWindAttackCount(0)
    PlayerStatTrackState.SetComboAttackCount(0)
end

function OnTriggerEnter()
    -- get text component then change text
    PlayerStatTrackState.SetPassedTrigger(true)
    missionTextComponent.text = missionText
end

function Update(dt)
    --Log("XXXXXXXXXXXXXXXXXXXXXXXXXXX  TIMES " ..  PlayerStatTrackState.GetFireAttackCount())
    -- if player use fire attack 3 times, disable blocking collider
    if PlayerStatTrackState.GetPassedTrigger() then
        if PlayerStatTrackState.GetFireAttackCount() >= 3 then
            SetActiveEntity(blockerCollider, false)
        end
    end
end