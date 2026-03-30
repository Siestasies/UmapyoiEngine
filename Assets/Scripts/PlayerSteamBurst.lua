-- PlayerSteamBurst.lua
-- Steam Burst - powerful fusion attack requiring elemental combo (Fire + Water or Water + Fire)

local PlayerStatTrackState = require("PlayerStatTrackState")

ExposedVars = {
    steamBurstAnimationName = "atk_fire_water",
    steamBurstSoundName = "atk_fire_water",
    vfxPrefab = "steam vfx.prefab",
    attackDuration = 0.7,
    vfxOffsetX = 25.0,
    vfxOffsetY = -8.0
}

-- State-local variables
local attackTimer = 0
local attackPerformed = false
local attackStat = nil
local animator = nil
local vfx = nil
local collider = nil

function state_enter(entity)
    Log("Player entered Steam Burst state")
    
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
    
    -- Check mana cost
    attackStat = GetSteamBurstAttackStat(player)
    if player.mMana < attackStat.manaCost then
        Log("Not enough mana for Steam Burst!")
        ChangeState(entity, "PlayerIdle")
        return
    end

    -- AttackIndex
    player.currAttackIndex = 5
    
    -- Consume mana
    player.mMana = player.mMana - attackStat.manaCost
    local transform = GetTransformFrom(EntityID)
    SpawnFeedback(transform.worldPosition.x, transform.worldPosition.y + 10, tostring(attackStat.manaCost), "manaspend")

    -- Set elemental combo state
    --player.lastElementUsed = ElementType.Steam
    --player.elementComboTimer = player.elementComboWindow
    
    -- Initialize attack
    attackTimer = attackDuration
    attackPerformed = false
    
    -- Play animation and sound
    animator.animator:Play(steamBurstAnimationName, true)
    audio = GetAudioComponent()
    audio:play(EntityID, "SteamBurst")

    --local children = GetChildrenList(EntityID)
    --if #children > 0 then
    --    SetActiveEntity(children[1], true)
    --    if HasAnimatorOn(children[1]) then
    --        vfx = GetAnimatorFrom(children[1])
    --        vfx.animator:Play(steamBurstAnimationName, true)
    --        local vfxTransform = GetTransformFrom(children[1])
    --        vfxTransform.position = Vec2(vfxOffsetX, vfxOffsetY)
    --    end
    --end

    -- vfx is not a child of the player anymore its a seperated prefab now
    --###################################################################################
    --#######################       VFX Prefab Spawning     #############################
    --###################################################################################
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

    local angle = math.deg(math.atan(dir.y, dir.x))
    if angle < 0 then angle = angle + 360 end

    -- Spawn offset in front of player
    local spawnOffset = 14.0  -- adjust this value to taste
    --local spawnPos = Vec2(myPos.x + dir.x * spawnOffset, myPos.y + dir.y * spawnOffset)
    local spawnPos = Vec2(myPos.x + spawnOffset, myPos.y)
    if transform.scale.x < 0 then
        spawnPos = Vec2(myPos.x - spawnOffset, myPos.y)
    end

    local prefab = SpawnPrefab(vfxPrefab, spawnPos)
    local projectile = GetProjectileFrom(prefab)

    local player = GetPlayer()
    projectile.mStats.damage = player.mAttackDamage

    -- Rotate the prefab to face the direction
    local prefabTransform = GetTransformFrom(prefab)
    if prefabTransform then
        --Log("XXXXXXXXXXX SCALE: " ..transform.scale.x)
        if transform.scale.x < 0 then
            prefabTransform.scale.x = -1
        end
    end

    if spriteComp then
        --spriteComp.flipX = (angle >= 90 and angle <= 270)
    end
    --###################################################################################
    --###################################################################################
    --###################################################################################
    
    -- Stop movement
    if HasRigidBody() then
        local rb = GetRigidBody()
        if rb then
            rb.velocity = Vec2(0, 0)
        end
    end
    
    --FaceTowardsMouse(entity)

    PlayerStatTrackState.incrSteamburstAttack()
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
    attackTimer = attackTimer - dt
    
    -- Perform attack at animation midpoint
    if not attackPerformed and attackTimer < (attackDuration * 0.4) then
        Log("Steam Burst Attack!")
        attackPerformed = true
        -- Activate Corresponding Collider
        --collider.shapes[attackStat.triggerColliderIndex+2].isActive = true
        -- Play explosion sound
        PlaySound(steamBurstSoundName, 0.9, 0)
    end
    
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
    Log("Player exited Steam Burst state")
    --if attackStat then 
    --    collider.shapes[attackStat.triggerColliderIndex+2].isActive = false
    --end
    StopSound(steamBurstSoundName);
end

function GetSteamBurstAttackStat(player)
    if not player then return nil end
    
    local attackStats = player.attackStats
    if attackStats then
        for i = 1, #attackStats do
            local attack = attackStats[i]
            if attack and attack.elementType == ElementType.Steam then
                return attack
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