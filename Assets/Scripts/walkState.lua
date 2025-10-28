--imports base class interfaces
local BaseState = require("base_state")

--declare new state which inherits from BaseState
local walkState = BaseState:new()
function walkState:enter()
    
end

function walkState:exit()

end

function walkState:update(dt)
    --insert implementation here
end

return newState