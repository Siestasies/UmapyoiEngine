local originalPosX
local originalScaleX

function Start()
    local transform = GetTransform()
    originalPosX = transform.position.x
    originalScaleX = transform.scale.x

    Log("[HB DEBUG] Start")
    Log("[HB DEBUG] originalPosX:" .. originalPosX)
    Log("[HB DEBUG] originalScaleX:" .. originalScaleX)
end
  
function Update(dt)

    local parentID = GetParent(EntityID)
    local enemy = GetEnemyFrom(parentID)

    if enemy then
    
        local currentHealth = enemy.mHealth
        local maxHealth = enemy.mMaxHealth

        local ratio = 0
        if maxHealth > 0 then
            ratio = currentHealth / maxHealth
        end

        -- clamp
        if ratio < 0 then ratio = 0 end
        if ratio > 1 then ratio = 1 end

        local transform = GetTransform()

        transform.scale.x = originalScaleX * ratio

        local lost = originalScaleX * (1 - ratio)
        transform.position.x = originalPosX - (lost / 2)

        -- if needed in your engine:
        -- SetTransform(transform)
    end
end
