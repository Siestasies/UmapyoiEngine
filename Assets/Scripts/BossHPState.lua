local BossHP = {
    currBossHP = 0,
    currElement = 0
}

function BossHP.GetBossHP()
    return BossHP.currBossHP
end

function BossHP.SetBossHP(value)
    BossHP.currBossHP = value
end

function BossHP.GetBossElement()
    return BossHP.currElement
end

function BossHP.SetBossElement(value)
    BossHP.currElement = value
end

return BossHP