function OnPointerClick()
    LoadScene("main_menu.scn")
    GetAudioComponent():play(EntityID, "Menu Click")
end