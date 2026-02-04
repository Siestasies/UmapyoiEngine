-- PlayerThrow.lua
-- Throw state - handles throwing explosive kunai or other projectiles

ExposedVars = {
    throwAnimationName = "throw",
    throwDuration = 0.4,
    projectilePrefab = "ExplosiveKunai.json",
    throwForce = 500.0
}

-- State-local variables
local throwTimer = 0
local projectileSpawned = false

function state_enter(entity)
    Log("Player entered Throw state")
    
    if not HasPlayer() then
        ChangeState(entity, "PlayerIdle")
        return
    end
    
    local player = GetPlayer()
    if not player then
        ChangeState(entity, "PlayerIdle")
        return
    end
    
    -- Check if stunned
    if player.isStunned then
        ChangeState(entity, "PlayerIdle")
        return
    end
    
    -- Note: Throwable inventory is commented out in Player.h
    -- This can be re-enabled when inventory system is active
    -- if player.throwableInventory.explosiveKunaiCount <= 0 then
    --     Log("No kunai available!")
    --     ChangeState(entity, "PlayerIdle")
    --     return
    -- end
    
    -- Initialize throw
    throwTimer = throwDuration
    projectileSpawned = false
    
    -- Stop movement
    if HasRigidBody() then
        local rb = GetRigidBody()
        if rb then
            rb.velocity = Vec2(0, 0)
        end
    end
    
    -- Face towards mouse
    FaceTowardsMouse(entity)
    
    -- Play throw animation and sound
    PlayAnimation(entity, throwAnimationName)
    PlaySound("throw", 0.7, 0)
end

function state_update(entity, dt)
    if not HasPlayer() then
        ChangeState(entity, "PlayerIdle")
        return
    end
    
    local player = GetPlayer()
    if not player then
        ChangeState(entity, "PlayerIdle")
        return
    end
    
    -- Update timer
    throwTimer = throwTimer - dt
    
    -- Spawn projectile at the peak of the throw animation
    if not projectileSpawned and throwTimer < (throwDuration * 0.6) then
        SpawnProjectile(entity, player)
        projectileSpawned = true
    end
    
    -- Throw complete
    if throwTimer <= 0 then
        -- Check for movement input
        local moveX = 0
        local moveY = 0
        
        if KeyDown(KEY_W) then moveY = moveY + 1 end
        if KeyDown(KEY_S) then moveY = moveY - 1 end
        if KeyDown(KEY_A) then moveX = moveX - 1 end
        if KeyDown(KEY_D) then moveX = moveX + 1 end
        
        if moveX ~= 0 or moveY ~= 0 then
            ChangeState(entity, "PlayerRun")
        else
            ChangeState(entity, "PlayerIdle")
        end
    end
end

function state_exit(entity)
    Log("Player exited Throw state")
    
    -- Reset state-local variables
    throwTimer = 0
    projectileSpawned = false
end

-- Spawn the projectile
function SpawnProjectile(entity, player)
    if not HasTransform() then return end
    
    local transform = GetTransform()
    if not transform then return end
    
    -- Calculate spawn position (slightly in front of player)
    local spawnOffset = 20.0
    local facingRight = true
    
    if HasSprite() then
        local sprite = GetSprite()
        if sprite then
            facingRight = not sprite.flipX
        end
    end
    
    local spawnPos = Vec2(
        transform.position.x + (facingRight and spawnOffset or -spawnOffset),
        transform.position.y
    )
    
    -- Calculate direction towards mouse
    local mousePos = GetMousePosition()
    local dx = mousePos.x - transform.position.x
    local dy = mousePos.y - transform.position.y
    local length = math.sqrt(dx * dx + dy * dy)
    
    local dirX = 1
    local dirY = 0
    
    if length > 0 then
        dirX = dx / length
        dirY = dy / length
    end
    
    -- Spawn projectile prefab
    local projectileId = SpawnPrefab(projectilePrefab, spawnPos)
    
    if projectileId ~= -1 then
        Log("Spawned projectile: " .. tostring(projectileId))
        
        -- Set projectile velocity using AddForce
        local direction = Vec2(dirX, dirY)
        
        -- Calculate rotation angle for the projectile
        local angle = math.atan2(dirY, dirX) * (180 / math.pi)
        
        AddForce(projectileId, spawnPos, direction, throwForce, angle)
        
        -- Decrement kunai count if inventory system is active
        -- player.throwableInventory.explosiveKunaiCount = player.throwableInventory.explosiveKunaiCount - 1
    else
        LogWarning("Failed to spawn projectile!")
    end
end

-- Helper function to face towards mouse
function FaceTowardsMouse(entity)
    if not HasTransform() then return end
    if not HasSprite() then return end
    
    local transform = GetTransform()
    local sprite = GetSprite()
    
    if not transform or not sprite then return end
    
    local mousePos = GetMousePosition()
    local myPos = transform.position
    
    if mousePos.x < myPos.x then
        sprite.flipX = true
    else
        sprite.flipX = false
    end
end
