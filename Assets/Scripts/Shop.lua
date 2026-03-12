local RNG = require("xorRNG")
local rng

local children
local cardText = {}
local cardIcon = {}

function Start()
    RNG.seed(math.random(1,1000000))
    rng = RNG.new()

    -- init references for icon and text, for editing later
    children = GetChildrenList(EntityID)

    local card1Children = GetChildrenList(children[3])
    --cardIcon[1] = card1Children[3].GetImage()
    cardText[1] = GetTextFrom(card1Children[4])

    local card2Children = GetChildrenList(children[4])
    --cardIcon[2] = card2Children[3].GetImage()
    cardText[2] = GetTextFrom(card2Children[4])

    local card3Children = GetChildrenList(children[5])
    --cardIcon[3] = card3Children[3].GetImage()
    cardText[3] = GetTextFrom(card3Children[4])

    -- generate RNG for stat and stat multipliers
    -- aka for icon and text
    local cardStatRNG
    local statMultiRNG
    local statMulti
    for i = 1, 3 do
        statMultiRNG = rng:randomRange(1,3)
        cardStatRNG = rng:randomRange(1,5)

        if statMultiRNG == 1 then
            statMulti = 15
        elseif statMultiRNG == 2 then
            statMulti = 20
        elseif statMultiRNG == 3 then
            statMulti = 25
        end

        -- BALANCE STATS HERE
        if cardStatRNG == 1 then
            --cardIcon[i].texturePath = ""
            cardText[i].text = "Increase Max HP by " .. statMulti

        elseif cardStatRNG == 2 then
            cardText[i].text = "Increase Move Speed by " .. statMulti

        elseif cardStatRNG == 3 then
            if (statMulti == 15) then
                statMulti = 7
            elseif (statMulti == 20) then
                statMulti = 10
            else
                statMulti = 12
            end
            cardText[i].text = "Increase Crit Dmg by " .. statMulti

        elseif cardStatRNG == 4 then
            if (statMulti == 15) then
                statMulti = 0.33
            elseif (statMulti == 20) then
                statMulti = 0.67
            else
                statMulti = 1.0
            end
            cardText[i].text = "Decrease Dash CD by " .. statMulti 

        elseif cardStatRNG == 5 then
            if (statMulti == 15) then
                statMulti = 5
            elseif (statMulti == 20) then
                statMulti = 7
            else
                statMulti = 10
            end
            cardText[i].text = "Decrease Mana cost by " .. statMulti
        end

        Log("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX     "..cardText[i].text)
    end
    --PauseGame(true)
end