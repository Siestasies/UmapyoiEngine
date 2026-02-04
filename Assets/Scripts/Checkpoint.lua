-- PlayerBase.lua
-- Base script for player entity - handles health, mana regen, and death transitions

ExposedVars = {
    checkpointID = 0,
    checkpointX = 0.0,
    checkpointY = 0.0
}

function Start()

end

function Update(dt)
  
end

function OnDestroy()
   
end

function OnTriggerEnter(other)
    if HasPlayerOn(other) then
        local player = GetPlayerFrom(other)
        if player then
            if not player.checkpointData.hasCheckpoint or player.checkpointData.checkpointID <= checkpointID then
                player.checkpointData.hasCheckpoint = true
                player.checkpointData.checkpointID = math.floor(checkpointID)
                player.checkpointData.checkpointX = checkpointX
                player.checkpointData.checkpointX = checkpointY
                 Log("Checkpoint Saved!")
            end
        end
    end
end
