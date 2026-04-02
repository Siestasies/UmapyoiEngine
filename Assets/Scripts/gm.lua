ExposedVars = {
    isBossRoom = true
}

local playerEntity = -1
local wasPaused = false
local gameOverTimer = 0.0
local pausedByEscape = false
local escConsumed = false

function Start()
    PauseGame(false)
end

function Update(dt)
    playerEntity = FindEntityWithComponent("Player")
    local player = GetPlayerFrom(playerEntity)
    local health = player.mHealth
    local paused = IsGamePause()

    -- If nothing is pausing the game, clear ESC state
    if not paused then
        pausedByEscape = false
        escConsumed = false
    end

    if not (KeyPressed(KEY_ESCAPE) or GetControllerButtonInput(BTN_START, BTN_PRESS, 0))then
        escConsumed = false
    end

    if (KeyPressed(KEY_ESCAPE) or GetControllerButtonInput(BTN_START, BTN_PRESS, 0)) and not escConsumed then
        escConsumed = true
        if pausedByEscape then
            pausedByEscape = false
            PauseGame(false)  -- ESC owned this pause, ESC releases it
        elseif not paused then
            pausedByEscape = true
            PauseGame(true)   -- nothing else is paused, ESC takes ownership
        end
        -- if externally paused (statue etc.), ESC does nothing
    end

    if IsEntityValid(playerEntity) then
        if health > 0 then
            if pausedByEscape then
                if not wasPaused then
                    toggleGroupLowpass("MASTER", true)
                    wasPaused = true
                end
                local child = GetChildren(EntityID, 1)
                SetActiveEntity(child, true)   -- pause menu
                return
            elseif not paused then
                -- only clean up when nothing else is pausing the game
                if wasPaused then
                    toggleGroupLowpass("MASTER", false)
                    wasPaused = false
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
                SetActiveEntity(child, i == 3)
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
                SetActiveEntity(child, true)
                gameOverTimer = 0.0
            end
        end
    end
end