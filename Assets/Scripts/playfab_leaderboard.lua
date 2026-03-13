

function Start()

    local playTime = GetPlayTime()

    PlayFab_SubmitScore("CompletionTime", math.floor(playTime),
    function()

        Log("My play time : " .. playTime)

        PlayFab_GetAccountInfo(
        function(info)
            local myName = info.displayName
            Log("My display name: " .. myName)

            local l_entries = GetChildrenList(EntityID)
                PlayFab_GetLeaderboard("FastestCompletionTime", 100, 1,
                function(results) 
                    local placeholderNum = 5;
                    for i = 1, #results do

                        if i <= placeholderNum then
                            local entry_id = l_entries[i]
                            local childrenOfEntry = GetChildrenList(entry_id)
                            local nameText = GetTextFrom(childrenOfEntry[1])
                            local timeText = GetTextFrom(childrenOfEntry[2])
                            local rankText = GetTextFrom(childrenOfEntry[3])
                            nameText.text = results[i].displayName
                            timeText.text = tostring(math.tointeger(results[i].score) .. "s")
                            rankText.text = tostring(results[i].rank) .. ","
                        end

                        if results[i].displayName == myName then
                            local entry_id = GetChildren(l_entries[6], 0)
                            local childrenOfEntry = GetChildrenList(entry_id)
                            Log("player entry : " .. #results)
                            local nameText = GetTextFrom(childrenOfEntry[1])
                            local timeText = GetTextFrom(childrenOfEntry[2])
                            local rankText = GetTextFrom(childrenOfEntry[3])
                            nameText.text = results[i].displayName
                            timeText.text = tostring(math.tointeger(results[i].score) .. "s")
                            rankText.text = tostring(results[i].rank) .. ","
                        end
                    end
                end,
                function(msg)
                    Log("Failed: " .. msg)
                end
            )
            
        end,
        function(msg)
            Log("Failed: " .. msg)
        end
    )
    end,
    function(msg)
        Log("Failed: " .. msg)
    end)

end