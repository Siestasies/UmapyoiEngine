--copy and paste template for the states
--exposed vars for variable you want that can be editied in editor
ExposedVars = {
    attackRange = 5.0
}

local AttackCD = 0.0
local ChargeCD = 2.0

--takes in entity id from C++ to use in case needed
function state_enter(entity)
    if HasEnemy() then
        local enemy = GetEnemy()
        AttackCD = enemy.mAttackSpeed
    end
end

--takes in entity id from C++ to use in case needed
--uses dt for time updates etc
function state_update(entity, dt)
    AttackCD = AttackCD - dt
    --attack if no cd
    if AttackCD < 0 then
        --reset attack timer
        AttackCD = enemy.mAttackSpeed
        if HasEnemy() then
            local enemy = GetEnemy()
            --Do AoE dmg somehow idk
            DoAoE(entity, attackRange)
        end
    --
    elseif changeCD ~= 0 then
        --hover
    else

    end

end

--takes in entity id from C++ to use in case needed
function state_exit(entity)
    
end

--AoE damage function
--a bit janky but im doing my best :(
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