local playerEntity = -1

local maxMana = 0

function Start()
    -- Cross-entity access
    playerEntity = FindEntityWithComponent("Player")
    maxMana = GetPlayerFrom(playerEntity).mMaxMana
end

function Update(dt)
    local mana = GetPlayerFrom(playerEntity).mMana

    if HasChildren(EntityID, 0) then
        local bg = GetChildren(EntityID, 0)
        if HasChildren(bg, 0) then
            local fill = GetChildren(bg, 0)

            local amount = mana / maxMana

            if amount < 0 then amount = 0 end
            if amount > 1 then amount = 1 end

            GetImageFrom(fill).fillAmount = amount
        end
    end
end