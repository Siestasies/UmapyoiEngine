ExposedVars = {
    isBossRoom = true
}

local playerEntity = -1
local wasPaused = false
local gameOverTimer = 0.0  -- was 1.0, caused immediate trigger on first frame
local EscWasPressed = false

function Update(dt)
    playerEntity = FindEntityWithComponent("Player")
    local player = GetPlayerFrom(playerEntity)
    local health = player.mHealth
    local paused = IsGamePause()

    if KeyPressed(KEY_ESCAPE) then
        if EscWasPressed == false and paused == true then return end

        EscWasPressed = true
    end

    if IsEntityValid(playerEntity) then
        if health > 0 then
            if paused == true and EscWasPressed == true then
                toggleGroupLowpass("MASTER", true)
                wasPaused = true
                local child = GetChildren(EntityID, 1)
                SetActiveEntity(child, true) -- pause
                return
            else
                if wasPaused == true then
                    toggleGroupLowpass("MASTER", false)
                    wasPaused = false
                    EscWasPressed = false
                end
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

    if isBossRoom == true then
        gameOverTimer = gameOverTimer + dt
        if gameOverTimer >= 1 then
            local numEnemy = CountEntitiesWithComponent("Enemy")
            if numEnemy <= 0 then
                local child = GetChildren(EntityID, 2)
                SetActiveEntity(child, true) -- complete
                gameOverTimer = 0.0
            end
        end
    end
end