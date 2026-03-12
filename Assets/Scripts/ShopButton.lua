local stat
local numberStr
local number
local playerID
local parentObj

function Start()
    -- init references for icon and text, for editing later
    local children = GetChildrenList(EntityID)

    local cardTextComp = GetTextFrom(children[4])
    local cardText = cardTextComp.text

    local pattern_stat = "(.*) by (%d+%.?%d*)"
    stat, numberStr = string.match(cardText, pattern_stat)
    number = tonumber(numberStr)

    playerID = FindEntityWithComponent("Player")
    parentObj = GetParent(EntityID)
end

function GetFireSlashAttackStat(player)
    if not player then
        return nil
    end
    
    local attackStats = player.attackStats
    if attackStats then
        for i = 1, #attackStats do
            local attack = attackStats[i]
            if attack and attack.elementType == ElementType.Fire then
                return attack
            end
        end
    end
    
    return nil
end

function GetWaterSlashAttackStat(player)
    if not player then
        return nil
    end
    
    local attackStats = player.attackStats
    if attackStats then
        for i = 1, #attackStats do
            local attack = attackStats[i]
            if attack and attack.elementType == ElementType.Water then
                return attack
            end
        end
    end
    return nil
end

function GetWindSlashAttackStat(player)
    if not player then
        return nil
    end
    
    local attackStats = player.attackStats
    if attackStats then
        for i = 1, #attackStats do
            local attack = attackStats[i]
            if attack and attack.elementType == ElementType.Wind then
                return attack
            end
        end
    end
    return nil
end

function OnClick()
    player = GetPlayerFrom(playerID)

    -- BALANCED IN Shop.lua
    if stat == "Increase Max HP" then
        player.mMaxHealth = player.mMaxHealth + number
    elseif stat == "Increase Move Speed" then
        player.mSpeed = player.mSpeed + number
    elseif stat == "Increase Crit Dmg" then
        player.mCritDamage = player.mCritDamage + number
    elseif stat == "Decrease Dash CD" then
        player.mDashCDMax = player.mDashCDMax - number
    elseif stat == "Decrease Mana cost" then
        local fireAttack = GetFireSlashAttackStat(player)
        fireAttack.manaCost = fireAttack.manaCost - number
        local waterAttack = GetWaterSlashAttackStat(player)
        waterAttack.manaCost = waterAttack.manaCost - number
        local windAttack = GetWindSlashAttackStat(player)
        windAttack.manaCost = windAttack.manaCost - number
    end

    SetActiveEntity(parentObj, false)
end

function OnPointerEnter()
end

function OnPointerExit()
end