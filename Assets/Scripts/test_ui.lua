function Start()
    local effects = GetEffects()
    if effects then
        effects:PlayAll()
    end
end