-- PlayerBase.lua
-- Base script for player entity - handles health, mana regen, and death transitions
local children
ExposedVars = {
    healAmount = 100
}

local isUsed = false

function Start()
    children = GetChildrenList(EntityID)
end

function Update(dt)
  
end

function OnDestroy()
    Log("Kappa Statue destroyed")
end

function OnTriggerEnter(other)
    if not isUsed and HasPlayerOn(other) then
        local player = GetPlayerFrom(other)
        if player then
            player.mHealth = math.floor(player.mHealth + healAmount)
            player.mHealth = math.floor(math.min(player.mMaxHealth, player.mHealth))

            isUsed = true
            
            --SetActiveEntity(children[1], true)
        end

        GetAudioComponent():play(EntityID,"KappaInteract")
        --PauseGame(true)
    end
end
