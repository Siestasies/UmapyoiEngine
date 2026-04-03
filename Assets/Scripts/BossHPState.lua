local BossHP = {
    currBossHP = 0,
}

function BossHP.GetBossHP()
    return BossHP.currBossHP
end

function BossHP.SetBossHP(value)
    BossHP.currBossHP = value
end

return BossHP