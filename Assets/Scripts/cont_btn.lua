function OnPointerClick()
    PauseGame(false)
    GetAudioComponent():play(EntityID, "Menu Click")
end