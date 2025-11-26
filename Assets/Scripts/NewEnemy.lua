--basically include
local StateMachine = require("StateMachine")
local WalkState = require("WalkState")
local ChaseState = require("ChaseState")
local IdleState = require("IdleState")
local AttackState = require("AttackState")
local DieState = require("DieState")

ExposedVars = {
    speed = 100.0,
    name = "unknown",
    isActive = true,

    mHealth = 100,
    mMaxHealth = 100,
    mHealthRegenRate = 1.0,

    mSpeed = 10.0,

    mAttackDamage = 10,
    mAttackSpeed = 1.0,
    mAttackRange = 20.0,
    mVisualRange = 20.0,
    mDefense = 5,
}
--reference to FSM
local fsm = nil

local playerEntity = -1

local isWalk = false;

function Start()
    thisEntity = GetEntity(EntityID)

    fsm = StateMachine:new(thisEntity)
    --add states here
    fsm:addState("WalkState", WalkState)
    fsm:addState("ChaseState", ChaseState)
    fsm:addState("IdleState", IdleState)
    fsm:addState("AttackState", AttackState)
    fsm:addState("DieState", DieState)
    --add whatever state else
    -- Set initial state
    fsm:changeState("IdleState")

    thisEntity.mHealth = 100;
    
    playerEntity = FindEntityWithComponent("Player")
end

function Update(dt)
    local transform = GetTransform(EntityID)
    if fsm then
        fsm:update(dt)
    end

    local rb = GetRigidBody()
    if rb then
        local isMoving = (rb.velocity.x ~= 0 or rb.velocity.y ~= 0)
        
        if isMoving then
            if not isWalk then
                PlayEntitySound(EntityID,"footsteps", true, 1.0)
                isWalk = true
            end
        else
            if isWalk then
                StopEntitySoundByName(EntityID,"footsteps")
                isWalk = false
            end
        end
    end


    if transform then 
        if KeyReleased(KEY_V) then 
            PlayOneShotAtEntity(EntityID,"explosion",1.0)
        end

    else
        Log("components are missing");
    end


    if KeyReleased(KEY_O) then 
        PlaySound("cave", 0.5, -1)
    end
    
end

function OnCollisionEnter(otherEntity)
    Log(name .. " -- Collision entered -- " .. otherEntity)
    local transform = GetTransform(EntityID)
    if transform then
        PlayOneShotAtEntity(EntityID, "hurt", 0.5)
    end
    transform = GetTransform(otherEntity)

end

function OnCollision(otherEntity)
    --Log(name .. " -- Collided -- " .. otherEntity)
end

function OnCollisionExit(otherEntity)
    --Log(name .. " -- Collided exit -- " .. otherEntity)
end