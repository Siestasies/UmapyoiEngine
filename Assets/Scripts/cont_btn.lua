function OnPointerClick()
    PauseGame(false)
    GetAudioComponent():Play(EntityID, "Menu Click")
end