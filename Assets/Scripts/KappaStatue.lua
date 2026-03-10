-- PlayerBase.lua
-- Base script for player entity - handles health, mana regen, and death transitions

ExposedVars = {
    healAmount = 100
}

function Start()
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
            
            LoadScene("level_1_v2.scn")
        end
    end
end
