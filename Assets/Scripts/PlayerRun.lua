-- PlayerRun.lua
-- Run/Movement state - handles WASD movement

ExposedVars = {
    runAnimationName = "walk"
}

local animator = nil

function state_enter(entity)
    Log("Player entered Run state")
    
    -- Play run animation
    if HasAnimator() then
        animator = GetAnimator()
    end

    animator.animator:Play("walk", false)
    GetAudioComponent():playFaded(EntityID, "footsteps", 0.5)
end

function state_update(entity, dt)
    if not HasPlayer() then return end
    if not HasRigidBody() then return end
    if not HasTransform() then return end
    
    local player = GetPlayer()
    local rb = GetRigidBody()
    local transform = GetTransform()
    
    if not player or not rb or not transform then return end
    
    -- Check if stunned
    if player.isStunned then
        rb.velocity = Vec2(0, 0)
        ChangeState(entity, "PlayerIdle")
        return
    end
    
    local targetAccel = Vec2(0, 0)

    local targetVel = Vec2(0, 0)
    local speed = 50

    local deadZone = 0.25
    if KeyDown(KEY_W) or GetControllerAxesInput(AXIS_LEFT_Y, 0) < -deadZone then targetVel.y = targetVel.y + speed end
    if KeyDown(KEY_S) or GetControllerAxesInput(AXIS_LEFT_Y, 0) > deadZone  then targetVel.y = targetVel.y - speed end
    if KeyDown(KEY_A) or GetControllerAxesInput(AXIS_LEFT_X, 0) < -deadZone then targetVel.x = targetVel.x - speed end
    if KeyDown(KEY_D) or GetControllerAxesInput(AXIS_LEFT_X, 0) > deadZone  then targetVel.x = targetVel.x + speed end

    if targetVel.x == 0 and targetVel.y == 0 then
        rb.velocity = Vec2(0, 0)
        ChangeState(entity, "PlayerIdle")
        return
    end

    -- Normalize for diagonal movement
    if targetVel.x ~= 0 and targetVel.y ~= 0 then
        targetVel = targetVel * 0.7071 -- 1/sqrt(2)
    end

    rb.velocity = targetVel
    
    -- Flip sprite based on direction
    if rb.velocity.x < 0 and transform.scale.x > 0 then
       transform.scale.x = -1.0 * transform.scale.x
    elseif rb.velocity.x > 0 and transform.scale.x < 0 then
        transform.scale.x = -1.0 * transform.scale.x
    end
    
    if KeyPressed(KEY_L) or 
        GetControllerButtonInput(BTN_A, BTN_PRESS, 0) then
        -- Check if player has enough mana for wind dash
        if CanUseElementalAttack(player, "wind") then
            ChangeState(entity, "PlayerWindDash")
            return
        else
            Log("Not enough mana for Wind Dash!")
            SpawnFeedback(transform.worldPosition.x, transform.worldPosition.y + 10, "Not enough mana for Wind Dash!", "warning")
        end
    end
    
    -- Check for Fire Slash (Q key or configurable)
    if KeyPressed(KEY_K) or 
        GetControllerButtonInput(BTN_B, BTN_PRESS, 0) then
        -- Check if player has enough mana for fire slash
        if CanUseElementalAttack(player, "fire") then
            ChangeState(entity, "PlayerFireSlash")
            return
        else
            Log("Not enough mana for Fire Slash!")
            SpawnFeedback(transform.worldPosition.x, transform.worldPosition.y + 10, "Not enough mana for Fire Slash!", "warning")
        end
    end
    
    -- Check for Water Slash (E key)
    if KeyPressed(KEY_J) or 
        GetControllerButtonInput(BTN_Y, BTN_PRESS, 0) then
        if CanUseElementalAttack(player, "water") then
            ChangeState(entity, "PlayerWaterSlash")
            return
        else
            Log("Not enough mana for Water Slash!")
            SpawnFeedback(transform.worldPosition.x, transform.worldPosition.y + 10, "Not enough mana for Water Slash!", "warning")
        end
    end
    
end

function state_exit(entity)
    GetAudioComponent():fadeOut(EntityID, "footsteps", 0.5)
    Log("Player exited Run state")
end

-- Helper function to check if elemental attack can be used
function CanUseElementalAttack(player, elementType)
    if not player then return false end
    
    -- Find the attack stats for this element
    local attackStats = player.attackStats
    if not attackStats then return false end
    
    for i = 1, #attackStats do
        local attack = attackStats[i]
        if attack then
            -- Check element type and mana cost
            if elementType == "fire" and attack.elementType == ElementType.Fire then
                return player.mMana >= attack.manaCost
            elseif elementType == "water" and attack.elementType == ElementType.Water then
                return player.mMana >= attack.manaCost
            elseif elementType == "wind" and attack.elementType == ElementType.Wind then
                return player.mMana >= attack.manaCost
            end
        end
    end
    
    return nil
end
