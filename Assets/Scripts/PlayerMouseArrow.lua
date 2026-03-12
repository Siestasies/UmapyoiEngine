--local playerTransform
local transform
local playerId

function Start()
    playerId = FindEntityWithComponent("Player")
    --playerTransform = GetTransformFrom(player)
    transform = GetTransform()
end

function Update(dt)
    local playerTf = GetTransformFrom(playerId)
    local collider = GetColliderFrom(playerId)

    if not playerTf then return end

    local shape = collider.shapes[1]
    local myPos = Vec2(playerTf.position.x, playerTf.position.y + shape.offset.y)

    local playerPos = myPos
    local mousePos = GetMouseWorldPosition()

    transform.position = playerPos

    -- direction from player to mouse
    local dx = mousePos.x - playerPos.x
    local dy = playerPos.y - mousePos.y

    local length = math.sqrt(dx*dx + dy*dy)
    if length == 0 then return end

    -- normalize
    dx = dx / length
    dy = dy / length
    
    local angle = math.atan(dy, -dx)

    if angle < 0 then
        angle = angle + (2 * math.pi)
    end

    --transform.scale.x = playerTf.scale.x * -1
    transform.rotation.x = angle + (math.pi / 2)
end