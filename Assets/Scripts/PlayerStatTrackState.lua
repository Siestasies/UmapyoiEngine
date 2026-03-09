local PlayerStatTrackState = {
    bFireAttackCount = 0,
    bWaterAttackCount = 0,
    bWindAttackCount = 0,
    comboAttackCount = 0,
    passedTrigger = 0
}

-- increment functions
function PlayerStatTrackState.incrFireAttack()
    Log("XXXXXXXXXXXXXXXXXXXXXXXXXXX  current trigger count " ..  PlayerStatTrackState.passedTrigger)
    Log("XXXXXXXXXXXXXXXXXXXXXXXXXXX  current fire attack count " ..  PlayerStatTrackState.bFireAttackCount)
    if PlayerStatTrackState.passedTrigger == 1 then
        PlayerStatTrackState.bFireAttackCount =
            PlayerStatTrackState.bFireAttackCount + 1
    end
end

function PlayerStatTrackState.incrWaterAttack()
    Log("XXXXXXXXXXXXXXXXXXXXXXXXXXX  current trigger count " ..  PlayerStatTrackState.passedTrigger)
    Log("XXXXXXXXXXXXXXXXXXXXXXXXXXX  current water attack count " ..  PlayerStatTrackState.bWaterAttackCount)
    if PlayerStatTrackState.passedTrigger == 3 then
        PlayerStatTrackState.bWaterAttackCount =
            PlayerStatTrackState.bWaterAttackCount + 1
    end
end

function PlayerStatTrackState.incrWindAttack()
    Log("XXXXXXXXXXXXXXXXXXXXXXXXXXX  current trigger count " ..  PlayerStatTrackState.passedTrigger)
    Log("XXXXXXXXXXXXXXXXXXXXXXXXXXX  current wind attack count " ..  PlayerStatTrackState.bWindAttackCount)
    if PlayerStatTrackState.passedTrigger == 2 then
        PlayerStatTrackState.bWindAttackCount =
            PlayerStatTrackState.bWindAttackCount + 1
    end
end

function PlayerStatTrackState.incrComboAttack()
    Log("XXXXXXXXXXXXXXXXXXXXXXXXXXX  current trigger count " ..  PlayerStatTrackState.passedTrigger)
    Log("XXXXXXXXXXXXXXXXXXXXXXXXXXX  current combo attack count " ..  PlayerStatTrackState.comboAttackCount)
    if PlayerStatTrackState.passedTrigger == 4 then
        PlayerStatTrackState.comboAttackCount =
            PlayerStatTrackState.comboAttackCount + 1
    end
end

function PlayerStatTrackState.incrPassedTrigger()
    PlayerStatTrackState.passedTrigger =
        PlayerStatTrackState.passedTrigger + 1
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