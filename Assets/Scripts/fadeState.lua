-- fade_state.lua
local fadeState = {
    isFading = false  -- Boolean flag to track fade state
}

-- functions to get current state
function fadeState.getFading()
    return fadeState.isFading
end

-- functions to set current state
function fadeState.setFading(x)
    fadeState.isFading = x
end

return fadeState