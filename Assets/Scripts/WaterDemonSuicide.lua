--copy and paste template for the states
--exposed vars for variable you want that can be editied in editor
ExposedVars = {
    explosionTimer = 2.0,
    damageRange = 5.0
}
local exploded = false

--takes in entity id from C++ to use in case needed
function state_enter(entity)

end

--takes in entity id from C++ to use in case needed
--uses dt for time updates etc
function state_update(entity, dt)
    explosionTimer = explosionTimer - dt
    --play death animation under the assumption that it takes the same as animation time
    if explosionTimer < 0 then
        --do AoE dmg
        DoAoE(entity, damageRange)
        exploded = true
    end

    if exploded == true then
        DestroyEntityWithChildren(entity)
    end
end

--takes in entity id from C++ to use in case needed
function state_exit(entity)
    
end

function DoAoE(entity, range)
    local playerId = FindEntityWithComponent("Player")
    if playerId ~= -1 then
        local player = GetEntity(playerId)
        if player then
            if HasEnemy() then
                local enemy = GetEnemy()
                if enemy then
                    local dist = GetPlayerDistance(entity)
                    if dist ~= math.huge and dist <= range  then
                        player.mHealth = player.mHealth - enemy.mAttackDamage
                    end
                end
            end
        end
    end
end

function GetPlayerDistance(entity)
    playerEntity = FindEntityWithComponent("Player")
    if not playerEntity then return math.huge end
    return Distance(GetTransform(entity).position, GetTransform(playerEntity).position)
end