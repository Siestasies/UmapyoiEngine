--basically include
local StateMachine = require("StateMachine")
local WalkState = require("WalkState")
local ChaseState = require("ChaseState")
local IdleState = require("IdleState")
local AttackState = require("AttackState")
local DieState = require("DieState")

ExposedVars = {
    name = "unknown",
    isActive = true,
}
--reference to FSM
local fsm = nil

--local playerEntity = -1

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
    
    thisEntity.playerEntity = FindEntityWithComponent("Player")
end

function Update(dt)
    local transform = GetTransform(EntityID)
    if fsm then
        fsm:update(dt)
    end

    local rb = GetRigidBody()
    if rb then
        local isMoving = (rb.velocity.x ~= 0 or rb.velocity.y ~= 0)
        
        -- if isMoving then
        --     if not isWalk then
        --         PlayEntitySound(EntityID,"footsteps", true, 1.0)
        --         isWalk = true
        --     end
        -- else
        --     if isWalk then
        --         StopEntitySoundByName(EntityID,"footsteps")
        --         isWalk = false
        --     end
        -- end
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

function HandleCollision(trigger)
    if thisEntity.playerEntity == trigger then
        local playerComp = GetPlayerFrom(thisEntity.playerEntity)
        if playerComp then
            OnHurt(thisEntity.playerEntity, playerComp.mAttackDamage)

            local transform = GetTransform(EntityID)
            if transform then
                PlayOneShotAtEntity(EntityID, "hurt", 0.5)
            end
        end
    end
end

function OnHurt(player, damage)
    -- damage handling logic here

    local enemy = GetEnemy()
    enemy.mHealth = enemy.mHealth - (damage - enemy.mDefense)

    Log("============================================================")
    Log("Player " .. player .. " took " .. damage .. " damage")
    Log("============================================================")
end

function OnTriggerEnter(otherEntity)
    HandleCollision(otherEntity)

end

function OnCollisionEnter(otherEntity)
    -- Collision logic here
end

function OnCollision(otherEntity)
    -- Collision logic here
end

function OnCollisionExit(otherEntity)
    -- Collision logic here
end