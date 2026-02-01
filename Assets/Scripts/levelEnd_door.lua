local levelEndTrigger
local rb
local tf
local targetPosX
local diff

ExposedVars = {
    speed = 2.0,
    moveBy = 5.0
}

function Start()
    local bruh = require("fadeState")
    levelEndTrigger = require("levelEndTrigger")
    rb = GetRigidBody()
    tf = GetTransform()
    targetPosX = tf.position.x + moveBy
    diff = tf.position.x - targetPosX
end

function Update(dt)
    if levelEndTrigger.getLevelEnd() then

        -- disable rigidbody/sprite
        --rb.isActive = false
        
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