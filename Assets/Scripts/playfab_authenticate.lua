function Start()

    local config = PlayFabConfigSerializer_Load("playfab_dev.json")
    local needToGenName = false

    if config.customId == "" then
        config.customId = PlayFab_GenerateUUID4()
        needToGenName = true
    end

    PlayFab_LoginWithCustomID(config.customId, true,
        function()
            Log("Player Logged in!")
           
            PlayFabConfigSerializer_Save("playfab_dev.json", config)

            if needToGenName then
                local subString = "Player " .. string.sub(config.customId, 1, 5)
                PlayFab_SetDisplayName(
                subString,
                function()
                end,
                function(msg)
                    Log("Login failed: " .. msg)
                end
                )
            end
            
        end,
        function(msg)
            Log("Login failed: " .. msg)
        end
    )
end