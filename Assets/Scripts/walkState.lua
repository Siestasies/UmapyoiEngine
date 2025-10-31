--imports base class interfaces
local BaseState = require("baseState")

local WalkState = {}
setmetatable(WalkState, {__index = BaseState})  -- inherit from BaseState
WalkState.__index = WalkState --for instances to fine walk state

--declare new state which inherits from BaseState
function WalkState:new(fsm, parent)
    local instance = BaseState.new(self, fsm, parent)
    return instance
end

function WalkState:enter()
    
end

function WalkState:exit()

end

function WalkState:update(dt)
    if not self.parent or not self.parent.isValid then
        return
    end
    local transform = self.parent.GetTransform()
    if self.parent.HasTransform() and self.parent.HasRigidBody() then
        -- local rb = GetRigidBody()
        -- local moveVec = Vec2.new(-1, 0)

        -- -- Apply movement
        -- if moveVec.x ~= 0 or moveVec.y ~= 0 then
        --     local targetAccel = moveVec * speed
        --     -- Smooth interpolation like player
        --     currentAccel = currentAccel + (targetAccel - currentAccel) * accelSmoothFactor * dt
        --     rb.acceleration = currentAccel
        -- else
        --     -- Smooth to zero
        --     currentAccel = currentAccel + (Vec2(0, 0) - currentAccel) * accelSmoothFactor * dt
        --     rb.acceleration = currentAccel
        --     hasLoggedMovement = false
        -- end
        transform.position.x = transform.position.x + dt * 100;
    end

    if KeyPressed(KEY_U) then
        Log("pressed")
        self.fsm:changeState("TestState")
    end
end

return WalkState