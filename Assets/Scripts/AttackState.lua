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
    AttackCD = 1 * self.parent:GetEnemy().mAttackSpeed
end

-- Called every frame while in this state
function AttackState:update(dt)
    local transform = self.parent:GetTransform()
    local playerTransform = GetTransformFrom(self.parent.playerEntity)
    --use Vec2 when passing function to C++ functions
    local dir = Vec2(playerTransform.position.x - transform.position.x, playerTransform.position.y - transform.position.y)

    --make fireball thingy
    local angle = math.atan(dir.y, dir.x)

    AttackCD = AttackCD - dt;
    if AttackCD < 0 then
        local entity = SpawnPrefab("fireball.prefab")
        AddForce(entity, Vec2(transform.position.x,transform.position.y), dir, 10.0, math.deg(angle))

        AttackCD = 1 * self.parent:GetEnemy().mAttackSpeed
    end

    if self.parent:GetEnemy().mHealth <= 0 then
        self.fsm:changeState("DieState")
    end

    --use MyVec2 when trying to use functions like lenght which arent bound to c++ but in the Vec2 lua scripts
    local distance = MyVec2.new(transform.position.x - playerTransform.position.x,transform.position.y - playerTransform.position.y)
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