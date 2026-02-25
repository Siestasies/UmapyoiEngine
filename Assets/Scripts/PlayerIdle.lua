-- PlayerIdle.lua
-- Idle state - waits for input to transition to other states

ExposedVars = {
    idleAnimationName = "idle"
}

local animator = nil

function state_enter(entity)
    Log("Player entered Idle state")
    
    -- Play idle animation
    
    if HasAnimator() then
        animator = GetAnimator()
    end

    animator.animator:Play(idleAnimationName, false)
end

function state_update(entity, dt)
    if not HasPlayer() then return end
    
    local player = GetPlayer()
    if not player then return end
    
    -- Check if stunned - can't do anything
    if player.isStunned then
        return
    end
    
    -- Check for movement input
    local moveX = 0
    local moveY = 0
    
    if KeyDown(KEY_W) then moveY = moveY + 1 end
    if KeyDown(KEY_S) then moveY = moveY - 1 end
    if KeyDown(KEY_A) then moveX = moveX - 1 end
    if KeyDown(KEY_D) then moveX = moveX + 1 end
    
    -- If there's movement input, transition to Run state
    if moveX ~= 0 or moveY ~= 0 then
        ChangeState(entity, "PlayerRun")
        return
    end
    
    -- Check for dash input (Shift key)
    if KeyPressed(KEY_SHIFT) then
        if player.mDashCD <= 0 then
            ChangeState(entity, "PlayerDash")
            return
        end
        Log("Dash Failed")
    end
    
    -- Check for basic attack input (Left mouse button)
    if MouseButtonPressed(MOUSE_LEFT) then
        ChangeState(entity, "PlayerAttack")
        return
    end

    local transform = GetTransformFrom(EntityID)

    if KeyPressed(KEY_R) then
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
    if KeyPressed(KEY_Q) then
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
    if KeyPressed(KEY_E) then
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
    Log("Player exited Idle state")
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