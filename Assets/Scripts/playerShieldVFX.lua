local animator    = nil
local initialized = false
local parentID    = nil
local shape       = nil
local isAlive     = true
local isDying     = false
local deathTimer  = 0.0
local deathDuration = 0.5
local lifeTime    = 8.0

function Start()
end

function Init()
    if initialized then return end
    if not HasAnimator() then return end

    animator = GetAnimator()
    parentID = GetParent(EntityID)

    -- Disable parent collider while shield is active
    if parentID then
        local collider = GetColliderFrom(parentID)
        if collider then
            shape = collider.shapes[1]
            shape.isActive = false
        end
    end

    local player = GetPlayerFrom(parentID)
    player.hasShield = true
    player.isShieldBroken = false

    initialized = true
end

-- Re-enable parent collider
function RestoreParentCollider()
    if not parentID then return end
    local collider = GetColliderFrom(parentID)
    if collider then
        collider.shapes[1].isActive = true
    end
end

function BeginDie()
    if isDying then return end
    Log("SHIELD: BeginDie called")

    local player = GetPlayerFrom(GetParent(EntityID))
    player.hasShield = false
    player.isShieldBroken = false

    isDying = true
    isAlive = false
    deathTimer = deathDuration
    animator.animator:Play("shield down", false)
end

function Update(dt)
    Init()
    if not initialized then return end

    animator = GetAnimator()

    -- Shield up -> shielding transition
    if isAlive
    and animator.animator:GetCurrentClip() == "shield up"
    and animator.animator:HasFinished() then
        if shape then
            local transform = GetTransform()
            transform.position = Vec2(0, shape.offset.y)
        end
        animator.animator:Play("shielding", true)
    end

    -- Tick lifetime and check for shield break
    if isAlive then
        lifeTime = lifeTime - dt

        local player = GetPlayerFrom(GetParent(EntityID))
        if lifeTime <= 0 or (player.hasShield and player.isShieldBroken) then
            BeginDie()
        end
    end

    -- Count down death timer then destroy
    if isDying then
        deathTimer = deathTimer - dt
        if deathTimer <= 0 then
            RestoreParentCollider()
            DestroyEntity(EntityID)
        end
    end
end

function OnDestroy()
    Log("SHIELD: OnDestroy called, isDying=" .. tostring(isDying) .. " deathTimer=" .. tostring(deathTimer))
    RestoreParentCollider()
end

function OnCollisionEnter(other)
end

function OnCollisionExit(other)
end

function NeutralizeProjectile(projEntity)
    -- Move offscreen and disable collider instead of DestroyEntity
    -- to avoid race conditions with the deletion queue
    local projTf = GetTransformFrom(projEntity)
    if projTf then
        projTf.position = Vec2(10000, 10000)
    end
    local projCollider = GetColliderFrom(projEntity)
    if projCollider then
        projCollider.shapes[1].isActive = false
    end
end

function HandleCollision(trigger)
    Log("SHIELD: HandleCollision trigger=" .. tostring(trigger) .. " hasProj=" .. tostring(HasProjectileOn(trigger)) .. " hasEnemy=" .. tostring(HasEnemyOn(trigger)))
    -- Block projectiles even while dying
    if HasProjectileOn(trigger) then
        if IsEntityValid(trigger) then
            NeutralizeProjectile(trigger)
        end
        if isAlive then
            BeginDie()
        end
    elseif HasEnemyOn(trigger) then
        if isAlive then
            BeginDie()
        end
    end
end

function OnTriggerEnter(other, triggerOwner)
    HandleCollision(triggerOwner)
end

function OnTriggerExit(other)
end
