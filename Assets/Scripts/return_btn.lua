function OnPointerClick()
    PauseGame(false)
    LoadScene("main_menu.scn")

    GetAudioComponent():play(EntityID, "Menu Click")
end