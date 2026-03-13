function OnPointerClick()

    StartPlayTime(false)
    PauseGame(false)
    LoadScene("leaderboard.scn")

    GetAudioComponent():play(EntityID, "Menu Click")
end