-- EnemyFSM.lua - Full structure, walkV2 tester only
ExposedVars = {
    name = "Enemy FSM Controller",
    isActive = true,
    sightRange = 400.0,
    attackRange = 150.0,
    healthThreshold = 0.3,
    lowHealthFlee = true,
    enableWalkV2 = true,       -- test toggle
}

local playerEntity = nil

function Start()
    playerEntity = FindEntityWithComponent("Player")
    Log("EnemyFSM controller started on " .. EntityID)
end

function Update(dt)
    if not ExposedVars.isActive then return end
    
    -- Health management
    UpdateHealth()
    
    -- Test: walkV2 only
    if ExposedVars.enableWalkV2 then
        ChangeState(EntityID, "walkV2")
    end
end

function UpdateHealth()
    local enemy = GetEnemy()
    if enemy and enemy.mHealth <= 0 then
        RequestFSMState(EntityID, "die")
    end
end

function GetPlayerDistance()
    if not playerEntity then return math.huge end
    return Distance(GetTransform(EntityID).position, GetTransform(playerEntity).position)
end

function OnTriggerEnter(otherEntity)
    if playerEntity == otherEntity then
        local playerComp = GetPlayerFrom(playerEntity)
        if playerComp then
            local enemy = GetEnemy()
            if enemy then
                enemy.mHealth = enemy.mHealth - playerComp.mAttackDamage
                PlayEntitySound(EntityID, "hurt", false, 0.8)
            end
        end
    end
end

-- Debug toggle
if KeyPressed(KEY_1) then 
    ExposedVars.enableWalkV2 = not ExposedVars.enableWalkV2 
    Log("walkV2 enabled: " .. tostring(ExposedVars.enableWalkV2))
end
