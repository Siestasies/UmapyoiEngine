function OnPointerClick()
    LoadScene("main_menu.scn")
    GetAudioComponent():play(Entity, "MenuClick")
end