local PlayerStatTrackState = {
    bFireAttackCount = 0,
    bWaterAttackCount = 0,
    bWindAttackCount = 0,
    comboAttackCount = 0,
    passedTrigger = false
}

-- increment functions
function PlayerStatTrackState.incrFireAttack()
    if PlayerStatTrackState.passedTrigger == true then
        PlayerStatTrackState.bFireAttackCount =
            PlayerStatTrackState.bFireAttackCount + 1
    end
end

function PlayerStatTrackState.incrWaterAttack()
    if PlayerStatTrackState.passedTrigger == true then
        PlayerStatTrackState.bWaterAttackCount =
            PlayerStatTrackState.bWaterAttackCount + 1
    end
end

function PlayerStatTrackState.incrWindAttack()
    if PlayerStatTrackState.passedTrigger == true then
        PlayerStatTrackState.bWindAttackCount =
            PlayerStatTrackState.bWindAttackCount + 1
    end
end

function PlayerStatTrackState.incrComboAttack()
    if PlayerStatTrackState.passedTrigger == true then
        PlayerStatTrackState.comboAttackCount =
            PlayerStatTrackState.comboAttackCount + 1
    end
end

-- Getter functions
function PlayerStatTrackState.GetFireAttackCount()
    return PlayerStatTrackState.bFireAttackCount
end

function PlayerStatTrackState.GetWaterAttackCount()
    return PlayerStatTrackState.bWaterAttackCount
end

function PlayerStatTrackState.GetWindAttackCount()
    return PlayerStatTrackState.bWindAttackCount
end

function PlayerStatTrackState.GetComboAttackCount()
    return PlayerStatTrackState.comboAttackCount
end

function PlayerStatTrackState.GetPassedTrigger()
    return PlayerStatTrackState.passedTrigger
end

-- Setter functions
function PlayerStatTrackState.SetFireAttackCount(value)
    PlayerStatTrackState.bFireAttackCount = value
end

function PlayerStatTrackState.SetWaterAttackCount(value)
    PlayerStatTrackState.bWaterAttackCount = value
end

function PlayerStatTrackState.SetWindAttackCount(value)
    PlayerStatTrackState.bWindAttackCount = value
end

function PlayerStatTrackState.SetComboAttackCount(value)
    PlayerStatTrackState.comboAttackCount = value
end

function PlayerStatTrackState.SetPassedTrigger(value)
    PlayerStatTrackState.passedTrigger = value
end

return PlayerStatTrackState