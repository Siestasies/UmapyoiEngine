-- PlayerSteamBurst.lua
-- Steam Burst - powerful fusion attack requiring elemental combo (Fire + Water or Water + Fire)

ExposedVars = {
    pyronadoAnimationName = "atk_fire_wind",
    pyronadoVFXAnimationName = "fire_wind",
    vfxPrefab = "pyronado vfx.prefab",
    attackDuration = 0.7,
    vfxOffsetX = 2.0,
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
    Log("Player entered Pyronado state")
    
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
    attackStat = GetPyronadoAttackStat(player)
    if player.mMana < attackStat.manaCost then
        Log("Not enough mana for Pyronado!")
        ChangeState(entity, "PlayerIdle")
        return
    end

    -- AttackIndex
    player.currAttackIndex = 6
    
    -- Consume mana
    player.mMana = player.mMana - attackStat.manaCost

    -- Set elemental combo state
    --player.lastElementUsed = ElementType.Steam
    --player.elementComboTimer = player.elementComboWindow
    
    -- Initialize attack
    attackTimer = attackDuration
    attackPerformed = false
    
    -- Play animation and sound
    animator.animator:Play(pyronadoAnimationName, true)
    audio = GetAudioComponent()
    audio:play(EntityID, pyronadoSoundName)

    -- vfx is not a child of the player anymore its a seperated prefab now
    --###################################################################################
    --#######################       VFX Prefab Spawning     #############################
    --###################################################################################

    local transform = GetTransform()
    local collider = GetCollider()

    local shape = collider.shapes[1]

    if not transform then return end

    local myPos = Vec2(transform.worldPosition.x, transform.worldPosition.y + shape.offset.y)

    local prefab = SpawnPrefab(vfxPrefab, myPos)
    local projectile = GetProjectileFrom(prefab)

    local player = GetPlayer()
    projectile.mStats.damage = player.mAttackDamage

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
    
    FaceTowardsMouse(entity)
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
    if not attackPerformed and attackTimer < (attackDuration * 0.9) then
        Log("Pyronado Attack!")
        attackPerformed = true
        -- Activate Corresponding Collider
        collider.shapes[attackStat.triggerColliderIndex+2].isActive = true
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
    Log("Player exited Pyronado state")

    if attackStat then 
        collider.shapes[attackStat.triggerColliderIndex+2].isActive = false
    end
    StopSound(pyronadoSoundName);
end

function GetPyronadoAttackStat(player)
    if not player then return nil end
    
    local attackStats = player.attackStats
    if attackStats then
        for i = 1, #attackStats do
            local attack = attackStats[i]
            if attack and attack.elementType == ElementType.Pyronado then
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