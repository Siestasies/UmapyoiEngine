--imports base class interfaces
local BaseState = require("baseState")

local WalkState = {}
setmetatable(WalkState, {__index = BaseState})  -- inherit from BaseState
WalkState.__index = WalkState --for instances to fine walk state

--declare new state which inherits from BaseState
function WalkState:new(fsm)
    local instance = BaseState.new(self, fsm) --calls parent constructor
    return instance
end

function WalkState:enter()
    
end

function WalkState:exit()

end

function WalkState:update(dt)
    --insert implementation here
    Log("IM WALKING HERE")
end

return WalkState