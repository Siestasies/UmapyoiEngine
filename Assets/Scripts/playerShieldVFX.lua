ExposedVars = {
    lifeTime = 5.0
}

local animator    = nil
local isAlive     = true
local isDying     = false
local initialized = false
local parentID    = nil
local shape       = nil

function Start()
end

function Init()
    if initialized then return end
    if not HasAnimator() then return end  -- retry next frame if not ready

    animator = GetAnimator()
    parentID = GetParent(EntityID)

    if parentID then
        local collider = GetColliderFrom(parentID)
        if collider then
            shape = collider.shapes[1]
        end
    end

    local player = GetPlayerFrom(parentID)

    player.hasShield = true
    player.isShieldBroken = false

    initialized = true
end

function BeginDie()
    if isDying then return end

    local playerID = GetParent(EntityID)
    local player = GetPlayerFrom(playerID)
    
    player.hasShield = false
    player.isShieldBroken = false

    isDying = true
    isAlive = false
    animator.animator:Play("shield down", true)
    --Log("WATER SHIELD: BeginDie called, playing shield down")
end

function Update(dt)
    Init()
    if not initialized then return end  -- don't run anything until ready

    animator = GetAnimator()

    --Log("WATER SHIELD clip: " .. tostring(animator.animator:GetCurrentClip()) .. " | finished: " .. tostring(animator.animator:HasFinished()) .. " | isDying: " .. tostring(isDying))

    -- Shield up -> shielding transition
    if isAlive and
    animator.animator:GetCurrentClip() == "shield up" and
    animator.animator:HasFinished() then
        if shape then
            local transform = GetTransform()
            transform.position = Vec2(0, shape.offset.y)
        end
        animator.animator:Play("shielding", true)
    end

    -- Only tick lifetime after fully initialized
    if isAlive then
        lifeTime = lifeTime - dt

        local playerID = GetParent(EntityID)
        local player = GetPlayerFrom(playerID)

        if lifeTime <= 0 then
            BeginDie()
        elseif player.hasShield and player.isShieldBroken then
            BeginDie()
        end
    end

    -- Only destroy after shield down has actually started AND finished
    if isDying and
    animator.animator:GetCurrentClip() == "shield down" and
    animator.animator:HasFinished() then
        Log("WATER SHIELD: destroying")
        DestroyEntity(EntityID)
    end
end

function OnDestroy()
end

function OnCollisionEnter(other)
end

function OnCollisionExit(other)
end

function HandleCollision(trigger)
    if not isAlive then return end

    if HasEnemyOn(trigger) then
        BeginDie()
    elseif HasProjectileOn(trigger) then
        BeginDie()
    end
end

function OnTriggerEnter(other, triggerOwner)
    HandleCollision(triggerOwner)
end

function OnTriggerExit(other)
end