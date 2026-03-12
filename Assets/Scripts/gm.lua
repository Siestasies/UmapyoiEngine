local playerEntity = -1
-- local levelEndState

-- function Start()
--     levelEndState = require("levelEndState")    
-- end

function Update(dt)
    playerEntity = FindEntityWithComponent("Player")
    local player = GetPlayerFrom(playerEntity)
    local health = player.mHealth
    local paused = IsGamePause()
    if IsEntityValid(playerEntity) then
        if health > 0 then
            if paused == true then
                toggleGroupLowpass("MASTER", true)
                local child = GetChildren(EntityID, 1)
                SetActiveEntity(child, true) -- pause
                return
            else
                toggleGroupLowpass("MASTER", false)

                local child = GetChildren(EntityID, 1)
                SetActiveEntity(child, false)
            end
        end
    end

    if IsEntityValid(playerEntity) then
        if health <= 0 then
            for i = 1, 4 do
                local child = GetChildren(EntityID, i)
                if i == 3 then
                    SetActiveEntity(child, true) -- game over
                else
                    SetActiveEntity(child, false)
                end
            end
            PauseGame(true)
            return
        end
    end

    -- if levelEndState.getLevelEnd() then
    --         local child = GetChildren(EntityID, 2)
    --         SetActiveEntity(child, true) -- complete
    --     else
    --         local child = GetChildren(EntityID, 2)
    --         SetActiveEntity(child, false) -- complete
    --     end
    -- end
end