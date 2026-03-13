-- PlayerWindDash.lua
-- Fire Slash elemental attack - applies burn and sets up elemental combo
local audio = nil
local PlayerStatTrackState = require("PlayerStatTrackState")

ExposedVars = {
    WindDashAnimationName = "atk_3",
    WindDashSoundName = "atk_3",
    ComboActivationFrame = 4
}

-- State-local variables
local attackTimer = 0
local attackPerformed = false
local dashDirection = Vec2(0, 0)
local originalInvulnerable = false
local attackStat = nil
local animator = nil
local collider = nil

function state_enter(entity)
    Log("Player entered Wind Dash state")
    
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

    if HasAnimator() then
        animator = GetAnimator()
    end
    
    if HasCollider() then
        collider = GetCollider()
    end

    FaceTowardsMouse(player)
    attackStat = GetWindDashAttackStat(player)
    
    -- Check mana cost
    if player.mMana < attackStat.manaCost then
        Log("Not enough mana for Wind Dash!")
        ChangeState(entity, "PlayerIdle")
        return
    end

    -- AttackIndex
    player.currAttackIndex = 4
    
    -- Consume mana
    player.mMana = math.floor(player.mMana - attackStat.manaCost)
    local transform = GetTransformFrom(EntityID)
    SpawnFeedback(transform.worldPosition.x, transform.worldPosition.y + 10, tostring(attackStat.manaCost), "manaspend")
    
    -- Set elemental combo state
    --player.lastElementUsed = ElementType.Wind
    --player.elementComboTimer = player.elementComboWindow
    
    -- Initialize attack/dash timer
    attackTimer = player.mDashDuration
    attackPerformed = false
    
    -- Store original invulnerability state and make player invulnerable
    originalInvulnerable = player.isInvulnerable
    player.isInvulnerable = true
    
    dashDirection = getDashDirection(player)
    
    -- Play animation and sound
    animator.animator:Play(WindDashAnimationName, true)
    audio = GetAudioComponent()
    audio:play(EntityID, "WindSlash")
    
    player.mDashCD = attackStat.attackCd

    -- Activate Corresponding Collider
    collider.shapes[attackStat.triggerColliderIndex+2].isActive = true
    PlaySound(WindDashSoundName, 0.9, 0)

    PlayerStatTrackState.incrWindAttack()
    Log("Wind Dash Attack!")

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
    
    -- Update timer
    attackTimer = attackTimer - dt

    -- Check for Fire Slash (Q key)
    if KeyPressed(KEY_Q) and animator.animator:GetCurrentFrame() >= ComboActivationFrame then
        if CanUseElementalAttack(player, "fire") then
            ChangeState(entity, "PlayerPyronado")
            return
        else
            Log("Not enough mana for Pyronado!")
            SpawnFeedback(transform.worldPosition.x, transform.worldPosition.y + 10, "Not enough mana for Pyronado!", "warning")            
        end
    end

    -- Check for Water Slash (E key)
    if KeyPressed(KEY_E) and animator.animator:GetCurrentFrame() >= ComboActivationFrame then
        if CanUseElementalAttack(player, "water") then
            ChangeState(entity, "PlayerWhirlpool")
            return
        else
            Log("Not enough mana for Whirlpool!")
            SpawnFeedback(transform.worldPosition.x, transform.worldPosition.y + 10, "Not enough mana for Whirlpool!", "warning")            
        end
    end

    --[[ Perform damage at attack midpoint
    if not attackPerformed and ColliderActivationFrame >= animator.animator:GetCurrentFrame() then
        Log("Wind Dash Attack!")
        attackPerformed = true
        -- Activate Corresponding Collider
        collider.shapes[attackStat.triggerColliderIndex+2].isActive = true
    end]]

    -- Apply attack/dash velocity
    local dashSpeed = player.mSpeed * player.mDashSpeed
    rb.velocity = Vec2(dashDirection.x * dashSpeed, dashDirection.y * dashSpeed)
    
    -- Attack finished
    if attackTimer <= 0 then
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
    Log("Player exited Wind Dash state")
    
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

    if attackStat then 
        collider.shapes[attackStat.triggerColliderIndex+2].isActive = false
    end

    local collider = GetCollider()
    if collider then
        shape = collider.shapes[1]
        shape.isActive = true;
    end

    StopSound(WindDashSoundName);
end

function GetWindDashAttackStat(player)
    if not player then return nil end
    
    local attackStats = player.attackStats
    if attackStats then
        for i = 1, #attackStats do
            local attack = attackStats[i]
            if attack and attack.elementType == ElementType.Wind then
                return attack
            end
        end
    end
    
    return nil
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

-- Helper function to dash towards mouse
function getDashDirection(player)


    local transform = GetTransform()
    local collider = GetCollider()

    local shape = collider.shapes[1]

    if not transform then return end

    local mousePos = GetMouseWorldPosition()
    local myPos = Vec2(transform.position.x, transform.position.y + shape.offset.y)

    -- Direction from player to mouse
    local dx = mousePos.x - myPos.x
    local dy = mousePos.y - myPos.y
    local len = math.sqrt(dx * dx + dy * dy)
    if len == 0 then len = 1 end
    local dir = Vec2(dx / len, dy / len)
    
    -- Normalize direction
    local length = math.sqrt(dir.x * dir.x + dir.y * dir.y)
    if length > 0 then
        dir = Vec2(dir.x / length, dir.y / length)
    end

    return dir
end