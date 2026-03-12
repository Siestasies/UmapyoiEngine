function OnPointerClick()
    PauseGame(false)
    LoadScene("main_menu.scn")

    GetAudioComponent():Play(EntityID, "Menu Click")
end