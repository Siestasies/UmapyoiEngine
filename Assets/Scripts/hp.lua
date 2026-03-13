local playerEntity = -1

local maxHealth = 0
local belowQuater = false;

function Update(dt)
    playerEntity = FindEntityWithComponent("Player")
    maxHealth = GetPlayerFrom(playerEntity).mMaxHealth
    
    local health = GetPlayerFrom(playerEntity).mHealth

    local segment = maxHealth / 4

    for i = 0, 3 do
        if HasChildren(EntityID, i) then
            local bg = GetChildren(EntityID, i)
            if HasChildren(bg, 0) then
                local fill = GetChildren(bg, 0)

                local startHealth = segment * i
                local amount = (health - startHealth) / segment

                if amount < 0 then amount = 0 end
                if amount > 1 then amount = 1 end

                GetImageFrom(fill).fillAmount = amount
            end
        end
    end

    if belowQuater == false and health <= segment then
        toggleGroupLowpass("MASTER", true)
        belowQuater = true
    elseif belowQuater == true and health > segment then
        toggleGroupLowpass("MASTER", false)
        belowQuater = false
    end
end