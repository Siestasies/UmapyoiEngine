local levelEndState
local col
local tf
local targetPosX
local diff

ExposedVars = {
    speed = 2.0,
    moveBy = 5.0
}

function Start()
    levelEndState = require("levelEndState")
    col = GetCollider()
    tf = GetTransform()
    targetPosX = tf.position.x + moveBy
    diff = tf.position.x - targetPosX
end

function Update(dt)
    -- if level end (aka no enemies anymore), open te doors (suppose to animate)
    -- allow for player to walk on trigger for next scene
    if levelEndState.getLevelEnd() then

        -- disable collider for player to walk thru
        local shape = col:GetPrimaryShape()
        shape.isActive = false
        
        -- animate -5
        if (diff < 0) then
            if (tf.position.x > targetPosX) then
                tf.position.x = tf.position.x - (dt * speed)
            end
            if (tf.position.x <= targetPosX) then
                tf.position.x = targetPosX
            end
        end

        -- animate +5
        if (diff > 0) then
            if (tf.position.x < targetPosX) then
                tf.position.x = tf.position.x + (dt * speed)
            end
            if (tf.position.x >= targetPosX) then
                tf.position.x = targetPosX
            end
        end
    end
end