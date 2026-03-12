function OnValueChanged()
    local slider = GetSlider()
    setMasterVolume(slider.value)
end