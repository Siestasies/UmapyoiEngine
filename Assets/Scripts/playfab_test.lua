function Start()

    PlayFab_LoginWithCustomID("my_player_id", true,
        function()
            Log("Player Logged in!")

            PlayFab_SubmitScore("CompletionTime", 400,
                function() Log("Score submitted!") end,
                function(msg) LogError(msg) end
            )

            PlayFab_GetLeaderboard("FastestCompletionTime", 10, 1,
                function(entries)
                    for i = 1, #entries do
                        Log(entries[i].rank .. ". " .. entries[i].entityId .. " - " .. entries[i].score)
                    end
                end
            )


        end,
        function(msg)
            Log("Login failed: " .. msg)
        end
    )

end