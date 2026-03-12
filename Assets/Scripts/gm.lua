local playerEntity = -1

function Start()
    -- Cross-entity access
    playerEntity = FindEntityWithComponent("Player")
end

function Update(dt)
    local player = GetPlayerFrom(playerEntity)
    if not player then
        playerEntity = FindEntityWithComponent("Player")
        return
    end
end