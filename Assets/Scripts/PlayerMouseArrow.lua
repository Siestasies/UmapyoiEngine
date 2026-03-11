local playerTransform
local transform

function Start()
    player = FindEntityWithComponent("Player")
    playerTransform = GetTransformFrom(player)
    transform = GetTransform()
end

function Update(dt)
    local playerPos = playerTransform.position
    local mousePos = GetMouseWorldPosition()

    -- direction from player to mouse
    local dx = mousePos.x - playerPos.x
    local dy = playerPos.y - mousePos.y

    local length = math.sqrt(dx*dx + dy*dy)
    if length == 0 then return end

    -- normalize
    dx = dx / length
    dy = dy / length
    
    -- rotate arrow to face mouse

    local angle = math.atan(dy, dx)

    if angle < 0 then
        angle = angle + (2 * math.pi)
    end

    transform.rotation.x = angle + (math.pi / 2)
end