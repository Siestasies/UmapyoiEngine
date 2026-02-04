function Start()
    local effects = GetEffects()
    if effects then
        effects:Play("Smaller")
        effects:Play("Bigger")
    end
end