--imports base class interfaces
local BaseState = require("baseState")

local TestState = {}
setmetatable(TestState, {__index = BaseState})  -- inherit from BaseState
TestState.__index = TestState --for instances to fine walk state

--declare new state which inherits from BaseState
function TestState:new(fsm)
    local instance = BaseState.new(self, fsm) --calls parent constructor
    return instance
end

function TestState:enter()
    Log("entered the testing zone B)")
end

function TestState:exit()

end

function TestState:update(dt)
    --insert implementation here
end

return TestState