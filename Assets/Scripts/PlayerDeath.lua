-- PlayerDeath.lua
-- Death state - handles player death animation, respawn, or game over

ExposedVars = {
    deathAnimationName = "die",
    deathDuration = 1.5,
    respawnDelay = 2.0,
    enableRespawn = true,
    gameOverSceneName = "main_menu.scn"
}

-- State-local variables
local deathTimer = 0
local respawnTimer = 0
local respawnTriggered = false

function state_enter(entity)
    Log("Player entered Death state")
    
    if not HasPlayer() then return end
    
    local player = GetPlayer()
    if not player then return end
    
    -- Ensure health is at 0
    player.mHealth = 0
    
    -- Initialize timers
    deathTimer = deathDuration
    respawnTimer = respawnDelay
    respawnTriggered = false
    
    -- Make player invulnerable (can't die again while dead)
    player.isInvulnerable = true
    player.isStunned = true
    
    -- Stop all movement
    if HasRigidBody() then
        local rb = GetRigidBody()
        if rb then
            rb.velocity = Vec2(0, 0)
        end
    end
    
    -- Play death animation and sound
    PlayAnimation(entity, deathAnimationName)
    --PlaySound("player_death", 1.0, 0)
    
    Log("Player has died!")
end

function state_update(entity, dt)
    if not HasPlayer() then return end
    
    local player = GetPlayer()
    if not player then return end
    
    -- Update death animation timer
    if deathTimer > 0 then
        deathTimer = deathTimer - dt
        Log("dtht" ..  tostring(deathTimer))
        
        if deathTimer <= 0 then
            Log("Death animation complete")
        end
    else
        -- After death animation, handle respawn or game over
        respawnTimer = respawnTimer - dt
        Log("rt" ..  tostring(respawnTimer))
        
        if respawnTimer <= 0 and not respawnTriggered then
            respawnTriggered = true
            
            if enableRespawn and player.checkpointData.hasCheckpoint then
                -- Respawn at checkpoint
                RespawnPlayer(entity, player)
            else
                -- No checkpoint or respawn disabled - game over
                HandleGameOver()
                Log("GameOver")
            end
        end
    end
end

function state_exit(entity)
    Log("Player exited Death state")
    
    -- Reset state-local variables
    deathTimer = 0
    respawnTimer = 0
    deathAnimationPlayed = false
    respawnTriggered = false
end

-- Respawn player at checkpoint
function RespawnPlayer(entity, player)
    if not player then return end
    
    Log("Respawning player at checkpoint...")
    
    -- Restore health and mana
    player.mHealth = player.mMaxHealth
    player.mMana = player.mMaxMana
    
    -- Clear status effects
    player.isStunned = false
    player.stunedTimer = 0
    player.isInvulnerable = true  -- Brief i-frames after respawn
    
    -- Reset elemental combo
    player.lastElementUsed = ElementType.None
    player.elementComboTimer = 0
    
    -- Reset attack index
    player.currAttackIndex = 0
    
    -- Move to checkpoint position
    if HasTransform() then
        local transform = GetTransform()
        if transform and player.checkpointData.hasCheckpoint then
            transform.position.x = player.checkpointData.checkpointX
            transform.position.y = player.checkpointData.checkpointY
            Log("Teleported to checkpoint: " .. tostring(transform.position.x) .. ", " .. tostring(transform.position.y))
        end
    end
    
    -- Play respawn sound
    PlaySound("respawn", 0.8, 0)
    
    -- Transition to idle state
    ChangeState(entity, "PlayerIdle")
end

-- Handle game over (no checkpoint or respawn disabled)
function HandleGameOver()
    Log("Game Over - No checkpoint available")
    
    -- Play game over sound
    PlaySound("game_over", 1.0, 0)
    
    -- Load game over scene after a brief delay
    -- Using LoadScene to transition
    if gameOverSceneName and gameOverSceneName ~= "" then
        LoadScene(gameOverSceneName)
    else
        Log("No game over scene specified!")
    end
end
