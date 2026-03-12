-- PlayerBase.lua
-- Base script for player entity - handles health, mana regen, and death transitions
local children
ExposedVars = {
    healAmount = 100
}

function Start()
    children = GetChildrenList(EntityID)
end

function Update(dt)
  
end

function OnDestroy()
    Log("Kappa Statue destroyed")
end

function OnTriggerEnter(other)
    if HasPlayerOn(other) then
        local player = GetPlayerFrom(other)
        if player then
            player.mHealth = math.floor(player.mHealth + healAmount)
            player.mHealth = math.floor(math.min(player.mMaxHealth, player.mHealth))
            
            SetActiveEntity(children[1], true)
        end
    end
end
