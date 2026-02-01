ExposedVars = {
    --empty for now
}
local enemy

function Start()
    if HasEnemy() then
        enemy = GetEnemy()
    end
end

function Update(dt)
    if enemy then
        if enemy.mHealth < 0 then
            ChangeState(EntityID, "WaterDemonSuicide")
        end
    end
end

function OnDestroy()
    
end

--optional use as needed
function OnCollisionEnter(other)
    
end

function OnCollisionExit(other)

end

function OnTriggerEnter(other)
    
end

function OnTriggerExit(other)
    
end
