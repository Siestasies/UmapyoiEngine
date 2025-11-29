-- Import the base state class
local BaseState = require("baseState")
local MyVec2 = require("Vec2")

-- Create the state class
local AttackState = {}

-- Set up inheritance from BaseState
setmetatable(AttackState, {__index = BaseState})
AttackState.__index = AttackState

-- Constructor - Creates a new instance of this state
function AttackState:new(fsm, parent)
    local instance = BaseState.new(self, fsm, parent)
    
    local AttackCD

    return instance
end

-- Called once when entering this state
function AttackState:enter()
    print("Entered AttackState")
    AttackCD = 1 / self.parent:GetEnemy().mAttackSpeed
end

-- Called every frame while in this state
function AttackState:update(dt)
    local transform = self.parent:GetTransform()

    local playerID = self.parent.playerEntity
    if GetParent(playerID) ~= -1 then 
        playerID = GetParent(playerID)
    end

    local playerTransform = GetTransformFrom(playerID)
    --use Vec2 when passing function to C++ functions
    local dir = Vec2(playerTransform.worldPosition.x - transform.worldPosition.x, playerTransform.worldPosition.y - transform.worldPosition.y)

    --make fireball thingy
    local angle = math.deg(math.atan(dir.y, dir.x))

    AttackCD = AttackCD - dt;
    if AttackCD < 0 then
        local entity = SpawnPrefab("fireball.prefab", Vec2(10000, 10000))
        -- SetActiveEntity(entity, false)
        -- SetActiveEntity(entity, true)

        GetProjectileFrom(entity).mDamage = self.parent:GetEnemy().mAttackDamage
        PlayEntitySound(self.parent.id, "fire_enemy_attack", false, 0.3);

        
        if angle < 0 then
            angle = angle + 360
        end

        AddForce(entity, Vec2(transform.worldPosition.x,transform.worldPosition.y), dir, GetProjectileFrom(entity).mSpeed, angle - 180)

        AttackCD = 1 / self.parent:GetEnemy().mAttackSpeed
    end

    if self.parent:GetEnemy().mHealth <= 0 then
        self.fsm:changeState("DieState")
    end

    --use MyVec2 when trying to use functions like lenght which arent bound to c++ but in the Vec2 lua scripts
    local distance = MyVec2.new(transform.worldPosition.x - playerTransform.worldPosition.x, transform.worldPosition.y - playerTransform.worldPosition.y)
    if distance:length() > 70 then
        self.fsm:changeState("IdleState")
    end
end

-- Called once when exiting this state
function AttackState:exit()
    
    
    print("Exited AttackState")
end

-- Return the state class so require() works
return AttackState