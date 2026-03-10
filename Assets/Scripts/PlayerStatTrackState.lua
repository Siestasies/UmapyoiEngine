local PlayerStatTrackState = {
    bFireAttackCount = 0,
    bWaterAttackCount = 0,
    bWindAttackCount = 0,

    pyronadoAttackCount = 0,
    whirlpoolAttackCount = 0,
    steamburstAttackCount = 0,
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
    if PlayerStatTrackState.passedTrigger == 1 then
        PlayerStatTrackState.bWaterAttackCount =
            PlayerStatTrackState.bWaterAttackCount + 1
    end
end

function PlayerStatTrackState.incrWindAttack()
    Log("XXXXXXXXXXXXXXXXXXXXXXXXXXX  current trigger count " ..  PlayerStatTrackState.passedTrigger)
    Log("XXXXXXXXXXXXXXXXXXXXXXXXXXX  current wind attack count " ..  PlayerStatTrackState.bWindAttackCount)
    if PlayerStatTrackState.passedTrigger == 1 then
        PlayerStatTrackState.bWindAttackCount =
            PlayerStatTrackState.bWindAttackCount + 1
    end
end

function PlayerStatTrackState.incrPyronadoAttack()
    Log("XXXXXXXXXXXXXXXXXXXXXXXXXXX  current trigger count " ..  PlayerStatTrackState.passedTrigger)
    Log("XXXXXXXXXXXXXXXXXXXXXXXXXXX  current combo attack count " ..  PlayerStatTrackState.pyronadoAttackCount)
    if PlayerStatTrackState.passedTrigger == 2 then
        PlayerStatTrackState.pyronadoAttackCount =
            PlayerStatTrackState.pyronadoAttackCount + 1
    end
end

function PlayerStatTrackState.incrWhirlpoolAttack()
    Log("XXXXXXXXXXXXXXXXXXXXXXXXXXX  current trigger count " ..  PlayerStatTrackState.passedTrigger)
    Log("XXXXXXXXXXXXXXXXXXXXXXXXXXX  current combo attack count " ..  PlayerStatTrackState.whirlpoolAttackCount)
    if PlayerStatTrackState.passedTrigger == 2 then
        PlayerStatTrackState.whirlpoolAttackCount =
            PlayerStatTrackState.whirlpoolAttackCount + 1
    end
end

function PlayerStatTrackState.incrSteamburstAttack()
    Log("XXXXXXXXXXXXXXXXXXXXXXXXXXX  current trigger count " ..  PlayerStatTrackState.passedTrigger)
    Log("XXXXXXXXXXXXXXXXXXXXXXXXXXX  current combo attack count " ..  PlayerStatTrackState.steamburstAttackCount)
    if PlayerStatTrackState.passedTrigger == 2 then
        PlayerStatTrackState.steamburstAttackCount =
            PlayerStatTrackState.steamburstAttackCount + 1
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

function PlayerStatTrackState.GetPyronadoAttackCount()
    return PlayerStatTrackState.pyronadoAttackCount
end

function PlayerStatTrackState.GetWhirlpoolAttackCount()
    return PlayerStatTrackState.whirlpoolAttackCount
end

function PlayerStatTrackState.GetSteamburstAttackCount()
    return PlayerStatTrackState.steamburstAttackCount
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

function PlayerStatTrackState.SetPyronadoAttackCount(value)
    PlayerStatTrackState.pyronadoAttackCount = value
end

function PlayerStatTrackState.SetWhirlpoolAttackCount(value)
    PlayerStatTrackState.whirlpoolAttackCount = value
end

function PlayerStatTrackState.SetSteamburstAttackCount(value)
    PlayerStatTrackState.steamburstAttackCount = value
end

function PlayerStatTrackState.SetPassedTrigger(value)
    PlayerStatTrackState.passedTrigger = value
end

return PlayerStatTrackState