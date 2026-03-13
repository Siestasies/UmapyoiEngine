function OnPointerClick()
    PauseGame(false)
    LoadScene("leaderboard.scn")

    GetAudioComponent():play(EntityID, "Menu Click")
end