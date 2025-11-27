-- Import the base state class
local BaseState = require("baseState")

-- Create the state class
local AttackState = {}

-- Set up inheritance from BaseState
setmetatable(AttackState, {__index = BaseState})
AttackState.__index = AttackState

-- Constructor - Creates a new instance of this state
function AttackState:new(fsm, parent)
    local instance = BaseState.new(self, fsm, parent)

    local attackCD = 5 * self.parent:GetEnemy().mAttackSpeed

    return instance
end

-- Called once when entering this state
function AttackState:enter()
    print("Entered AttackState")
end

-- Called every frame while in this state
function AttackState:update(dt)
    local transform = self.parent:GetTransform()
    local playerTransform = self.parent.playerEntity:GetTransform()
    local dir = vec2.new(transform.position.x - playerTransform.position.x,transform.position.y - playerTransform.position.y)

    --make fireball thingy
    attackCD = attackCD - dt
    if attackCD < 0 then
        local entity = self.parent:CreateEntity()
        self.parent:GetTransformFrom(entity).rotation.x = 1.0
    end

    if self.parent:GetEnemy().mHealth <= 0 then
        self.fsm:changeState("DieState")
    end

    local distance = Vec2.new(transform.position.x - playerTransform.position.x,transform.position.y - playerTransform.position.y)
    
    if distance:length() > 20 then
        self.fsm:changeState("IdleState")
    end
end

-- Called once when exiting this state
function AttackState:exit()
    
    
    print("Exited AttackState")
end

-- Return the state class so require() works
return AttackState