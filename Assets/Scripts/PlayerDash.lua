-- PlayerDash.lua
-- Dash state - quick movement with invulnerability frames

local audio = nil

ExposedVars = {
    dashAnimationName = "dash",
    dashDuration = 1,
    --dashCooldown = 2.0
}

-- State-local variables (reset on state enter)
local dashTimer = 0
local dashDirection = Vec2(0, 0)
local originalInvulnerable = false

function state_enter(entity)
    Log("Player entered Dash state")
    
    if not HasPlayer() then
        ChangeState(entity, "PlayerIdle")
        return
    end
    
    local player = GetPlayer()
    if not player then
        ChangeState(entity, "PlayerIdle")
        return
    end
    
    -- Initialize dash timer
    dashTimer = player.mDashDuration
    if dashTimer <= 0 then
        dashTimer = dashDuration
    end
    
    -- Store original invulnerability state and make player invulnerable
    originalInvulnerable = player.isInvulnerable
    player.isInvulnerable = true
    
        -- Calculate dash direction based on input or facing direction
    local moveX = 0
    local moveY = 0
    
    --[[
    if KeyDown(KEY_W) then moveY = moveY + 1 end
    if KeyDown(KEY_S) then moveY = moveY - 1 end
    if KeyDown(KEY_A) then moveX = moveX - 1 end
    if KeyDown(KEY_D) then moveX = moveX + 1 end
    
    -- If no input, dash in facing direction
    if moveX == 0 and moveY == 0 then
        if HasSprite() then
            local sprite = GetSprite()
            if sprite then
                --moveX = sprite.flipX and -1 or 1
                local playerTransform = GetTransformFrom(EntityID)
                if playerTransform.scale.x <= 0 then
                    moveX = -1
                else
                    moveX = 1
                end
            else
                moveX = 1
            end
        else
            moveX = 1
        end
    end
    
    -- Normalize direction
    local length = math.sqrt(moveX * moveX + moveY * moveY)
    if length > 0 then
        dashDirection = Vec2(moveX / length, moveY / length)
    else
        dashDirection = Vec2(1, 0)
    end

    ]]

    dashDirection = getDashDirection(player)
    
    -- Play dash animation
    PlayAnimation(entity, dashAnimationName)
    
    -- Play dash sound
    audio = GetAudioComponent()
    audio:play(EntityID, "PlayerDash")

    player.mDashCD = player.mDashCDMax;

    FaceTowardsMouse(entity)

    local collider = GetCollider()
    if collider then
        shape = collider.shapes[1]
        shape.isActive = false;
    end
end

function state_update(entity, dt)
    if not HasPlayer() then
        ChangeState(entity, "PlayerIdle")
        return
    end
    if not HasRigidBody() then
        ChangeState(entity, "PlayerIdle")
        return
    end
    
    local player = GetPlayer()
    local rb = GetRigidBody()
    
    if not player or not rb then
        ChangeState(entity, "PlayerIdle")
        return
    end
    
    -- Update dash timer
    dashTimer = dashTimer - dt
    
    -- Apply dash velocity
    local dashSpeed = player.mSpeed * player.mDashSpeed
    rb.velocity = Vec2(dashDirection.x * dashSpeed, dashDirection.y * dashSpeed)
    
    -- Check if dash is complete
    if dashTimer <= 0 then
        -- Return to appropriate state based on input
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
    Log("Player exited Dash state")
    
    if not HasPlayer() then return end
    
    local player = GetPlayer()
    if not player then return end
    
    -- Restore invulnerability state (keep i-frames briefly after dash)
    -- Player component will handle the invulnerability timer
    player.isInvulnerable = originalInvulnerable
    
    -- Stop dash velocity
    if HasRigidBody() then
        local rb = GetRigidBody()
        if rb then
            rb.velocity = Vec2(0, 0)
        end
    end
    
    -- Reset state-local variables
    dashTimer = 0
    dashDirection = Vec2(0, 0)
    local collider = GetCollider()
    if collider then
        shape = collider.shapes[1]
        shape.isActive = true;
    end
end

-- Helper function to dash towards mouse
function getDashDirection(player)
    if not HasTransform() then return end
    if not HasSprite() then return end
    
    local transform = GetTransform()
    --local sprite = GetSprite()
    
    if not transform then return end
    
    local mousePos = GetMouseWorldPosition()
    local myPos = transform.position
    local direction = Vec2(1, 0)
    
    -- Determine direction based on mouse position and player postion
    direction = mousePos - myPos
    
    -- Normalize direction
    local length = math.sqrt(direction.x * direction.x + direction.y * direction.y)
    if length > 0 then
        direction = Vec2(direction.x / length, direction.y / length)
    end

    return direction
end

-- Helper function to face towards mouse
function FaceTowardsMouse(entity)
    if not HasTransform() then return end
    if not HasSprite() then return end
    
    local transform = GetTransform()
    --local sprite = GetSprite()
    
    if not transform then return end
    
    local mousePos = GetMouseWorldPosition()
    local myPos = transform.position
    
    -- Determine facing direction based on mouse position
    if mousePos.x < myPos.x and transform.scale.x > 0 then
        --sprite.flipX = true
       transform.scale.x = -1.0 * transform.scale.x
    elseif mousePos.x > myPos.x and transform.scale.x < 0 then
        --sprite.flipX = false
        --myScale.x = 1.0 * myScale.x
        transform.scale.x = -1.0 * transform.scale.x
    end
end