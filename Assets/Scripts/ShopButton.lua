local stat
local numberStr
local number
local player
local parentObj

function Start()
    -- init references for icon and text, for editing later
    local children = GetChildrenList(EntityID)

    local cardTextComp = GetTextFrom(children[4])
    local cardText = cardTextComp.text

    local pattern_stat = "(.*) by (%d+)"
    stat, numberStr = string.match(cardText, pattern_stat)
    number = tonumber(numberStr)

    --player = GetPlayer() PROBLEM HERE
    parentObj = GetParent(EntityID)
end

function OnClick()
    if stat == "Increase Max HP" then
        --player.fireSlash_manaCost = 10
    elseif stat == "Increase Move Speed" then
        --player.fireSlash_manaCost = 10
    elseif stat == "Increase Crit Dmg" then
        --player.fireSlash_manaCost = 10
    elseif stat == "Decrease Dash CD" then
        --player.fireSlash_manaCost = 10
    elseif stat == "Decrease Mana cost" then
        --player.fireSlash_manaCost = 10
    end

    SetActiveEntity(parentObj, false)
end

function OnPointerEnter()
end

function OnPointerExit()
end