-- ====================================================================
-- COMPREHENSIVE LUA SCRIPTING SYSTEM TEST
-- Based on actual LuaScriptingSystem implementation
-- ====================================================================

ExposedVars = {
    speed = 100.0,
    name = "TestEntity",
    isActive = true,
    moveDistance = 50.0,
    testCounter = 0,
    rotationSpeed = 45.0
}

local playerEntity = -1
local allEnemies = {}
local hasLoggedMovement = false

local currentAccel = Vec2(0, 0)
local accelSmoothFactor = 15.0

function Start()
    Log("========================================")
    Log("COMPREHENSIVE TEST SCRIPT STARTED")
    Log("========================================")
    
    -- ================================================================
    -- TEST 1: Basic Entity Information
    -- ================================================================
    Log("\n[TEST 1] Entity Information:")
    Log("  My Entity ID: " .. EntityID)
    Log("  My Name: " .. name)
    Log("  My Speed: " .. speed)
    Log("  Is Active: " .. tostring(isActive))
    Log("  Total entities in scene: " .. GetEntityCount())
    Log("  Delta Time: " .. GetDeltaTime())
    
    -- ================================================================
    -- TEST 2: My Components (Using HasXXX and GetXXX)
    -- ================================================================
    Log("\n[TEST 2] My Components:")
    
    -- Transform
    if HasTransform() then
        local tf = GetTransform()
        Log("  ✓ Transform:")
        Log("    Position: (" .. tf.position.x .. ", " .. tf.position.y .. ")")
        Log("    Rotation: (" .. tf.rotation.x .. ", " .. tf.rotation.y .. ")")
        Log("    Scale: (" .. tf.scale.x .. ", " .. tf.scale.y .. ")")
    else
        LogWarning("  ✗ No Transform component")
    end
    
    -- RigidBody
    if HasRigidBody() then
        local rb = GetRigidBody()
        Log("  ✓ RigidBody:")
        Log("    Velocity: (" .. rb.velocity.x .. ", " .. rb.velocity.y .. ")")
        Log("    Acceleration: (" .. rb.acceleration.x .. ", " .. rb.acceleration.y .. ")")
        Log("    Accel Strength: " .. rb.accel_strength)
        Log("    Friction Coeff: " .. rb.fric_coeff)
    else
        Log("  ✗ No RigidBody component")
    end
    
    -- Sprite
    if HasSprite() then
        local sprite = GetSprite()
        Log("  ✓ Sprite:")
        Log("    Texture: " .. sprite.textureName)
        Log("    Render Layer: " .. sprite.renderLayer)
        Log("    Flip X: " .. tostring(sprite.flipX))
        Log("    Flip Y: " .. tostring(sprite.flipY))
    else
        Log("  ✗ No Sprite component")
    end
    
    -- Collider (Using direct vector access)
    if HasCollider() then
        local collider = GetCollider()
        Log("  ✓ Collider:")
        Log("    Default Layer: " .. collider.defaultLayer)
        Log("    Default Mask: " .. collider.defaultMask)
        Log("    Show BBox: " .. tostring(collider.showBBox))
        
        -- Access shapes vector directly using size() method
        local shapeCount = collider.shapes:size()
        Log("    Shape Count: " .. shapeCount)
        
        -- Iterate through shapes (1-indexed in Lua)
        for i = 1, shapeCount do
            local shape = collider.shapes[i]
            if shape then
                Log("    Shape " .. i .. ":")
                Log("      Size: (" .. shape.size.x .. ", " .. shape.size.y .. ")")
                Log("      Offset: (" .. shape.offset.x .. ", " .. shape.offset.y .. ")")
                Log("      Purpose: " .. shape.purpose)
                Log("      Layer: " .. shape.layer)
                Log("      Mask: " .. shape.colliderMask)
                Log("      Active: " .. tostring(shape.isActive))
                Log("      Auto-fit: " .. tostring(shape.autoFitToSprite))
            end
        end
        
        -- Access bounds vector directly
        local boundsCount = collider.bounds:size()
        Log("    Bounds Count: " .. boundsCount)
        for i = 1, boundsCount do
            local bounds = collider.bounds[i]
            if bounds then
                local width = bounds.max.x - bounds.min.x
                local height = bounds.max.y - bounds.min.y
                Log("    Bounds " .. i .. ": Size(" .. width .. " x " .. height .. ")")
            end
        end
        
        -- Test GetPrimaryShape (returns reference)
        local primaryShape = collider:GetPrimaryShape()
        Log("    Primary Shape Size: (" .. primaryShape.size.x .. ", " .. primaryShape.size.y .. ")")
        
        -- Test GetPrimaryBounds (returns reference)
        local primaryBounds = collider:GetPrimaryBounds()
        Log("    Primary Bounds: Min(" .. primaryBounds.min.x .. ", " .. primaryBounds.min.y .. 
            ") Max(" .. primaryBounds.max.x .. ", " .. primaryBounds.max.y .. ")")
        
        -- Test GetEffectiveLayer and GetEffectiveMask
        local effectiveLayer = collider:GetEffectiveLayer(0)  -- C++ side is 0-indexed
        local effectiveMask = collider:GetEffectiveMask(0)
        Log("    Effective Layer (shape 0): " .. effectiveLayer)
        Log("    Effective Mask (shape 0): " .. effectiveMask)
    else
        Log("  ✗ No Collider component")
    end
    
    -- Player component (if exists)
    if HasPlayer() then
        local player = GetPlayer()
        Log("  ✓ Player:")
        Log("    Speed: " .. player.mSpeed)
    end
    
    -- Enemy component (if exists)
    if HasEnemy() then
        local enemy = GetEnemy()
        Log("  ✓ Enemy:")
        Log("    Speed: " .. enemy.mSpeed)
    end
    
    -- Camera component (if exists)
    if HasCamera() then
        local camera = GetCamera()
        Log("  ✓ Camera:")
        Log("    Zoom: " .. camera.zoom)
        Log("    Follow Player: " .. tostring(camera.followPlayer))
    end
    
    -- ================================================================
    -- TEST 3: Entity Queries (Global Functions)
    -- ================================================================
    Log("\n[TEST 3] Entity Queries:")
    
    -- FindEntityWithComponent (finds first entity with component)
    playerEntity = FindEntityWithComponent("Player")
    if playerEntity ~= -1 then
        Log("  ✓ Found Player entity: " .. playerEntity)
        Log("    Is valid: " .. tostring(IsEntityValid(playerEntity)))
    else
        Log("  ✗ No Player entity found")
    end
    
    -- FindEntitiesWithComponent (finds all entities with component)
    allEnemies = FindEntitiesWithComponent("Enemy")
    Log("  Found " .. #allEnemies .. " enemy entities")
    for i, enemyId in ipairs(allEnemies) do
        Log("    Enemy " .. i .. ": ID = " .. enemyId)
    end
    
    -- ================================================================
    -- TEST 4: Cross-Entity Access (Direct Functions: GetXXXFrom)
    -- ================================================================
    Log("\n[TEST 4] Cross-Entity Access (Direct Functions):")
    
    if playerEntity ~= -1 then
        -- HasTransformOn + GetTransformFrom
        if HasTransformOn(playerEntity) then
            local playerTf = GetTransformFrom(playerEntity)
            Log("  ✓ Player Transform:")
            Log("    Position: (" .. playerTf.position.x .. ", " .. playerTf.position.y .. ")")
            Log("    Scale: (" .. playerTf.scale.x .. ", " .. playerTf.scale.y .. ")")
        end
        
        -- HasRigidBodyOn + GetRigidBodyFrom
        if HasRigidBodyOn(playerEntity) then
            local playerRb = GetRigidBodyFrom(playerEntity)
            Log("  ✓ Player RigidBody:")
            Log("    Velocity: (" .. playerRb.velocity.x .. ", " .. playerRb.velocity.y .. ")")
            Log("    Acceleration: (" .. playerRb.acceleration.x .. ", " .. playerRb.acceleration.y .. ")")
        end
        
        -- HasSpriteOn + GetSpriteFrom
        if HasSpriteOn(playerEntity) then
            local playerSprite = GetSpriteFrom(playerEntity)
            Log("  ✓ Player Sprite: " .. playerSprite.textureName)
        end
        
        -- HasColliderOn + GetColliderFrom
        if HasColliderOn(playerEntity) then
            local playerCollider = GetColliderFrom(playerEntity)
            Log("  ✓ Player Collider:")
            Log("    Shape count: " .. playerCollider.shapes:size())
            Log("    Show BBox: " .. tostring(playerCollider.showBBox))
        end
        
        -- HasPlayerOn (checks for Player component)
        if HasPlayerOn(playerEntity) then
            local playerComp = GetPlayerFrom(playerEntity)
            Log("  ✓ Entity has Player component (speed: " .. playerComp.mSpeed .. ")")
        end
        
        -- HasCameraOn + GetCameraFrom
        if HasCameraOn(playerEntity) then
            local playerCamera = GetCameraFrom(playerEntity)
            Log("  ✓ Player has Camera (zoom: " .. playerCamera.zoom .. ")")
        end
    end
    
    -- Test with enemies
    if #allEnemies > 0 then
        local enemyId = allEnemies[1]
        Log("\n  Testing Enemy " .. enemyId .. ":")
        
        if HasEnemyOn(enemyId) then
            local enemyComp = GetEnemyFrom(enemyId)
            Log("    ✓ Has Enemy component (speed: " .. enemyComp.mSpeed .. ")")
        end
        
        if HasTransformOn(enemyId) then
            local enemyTf = GetTransformFrom(enemyId)
            Log("    ✓ Enemy position: (" .. enemyTf.position.x .. ", " .. enemyTf.position.y .. ")")
        end
    end
    
    -- ================================================================
    -- TEST 5: Cross-Entity Access (Entity Wrapper: GetEntity)
    -- ================================================================
    Log("\n[TEST 5] Cross-Entity Access (Entity Wrapper):")
    
    if playerEntity ~= -1 then
        local playerWrapper = GetEntity(playerEntity)
        if playerWrapper.isValid then
            Log("  ✓ Player entity wrapper is valid (ID: " .. playerWrapper.id .. ")")
            
            -- Test HasXXX methods
            if playerWrapper:HasTransform() then
                local tf = playerWrapper:GetTransform()
                Log("    Has Transform at (" .. tf.position.x .. ", " .. tf.position.y .. ")")
            end
            
            if playerWrapper:HasRigidBody() then
                local rb = playerWrapper:GetRigidBody()
                Log("    Has RigidBody (vel: " .. rb.velocity.x .. ", " .. rb.velocity.y .. ")")
            end
            
            if playerWrapper:HasSprite() then
                local sprite = playerWrapper:GetSprite()
                Log("    Has Sprite (" .. sprite.textureName .. ")")
            end
            
            if playerWrapper:HasCollider() then
                local collider = playerWrapper:GetCollider()
                Log("    Has Collider (" .. collider.shapes:size() .. " shapes)")
            end
            
            if playerWrapper:HasPlayer() then
                local player = playerWrapper:GetPlayer()
                Log("    Has Player component (speed: " .. player.mSpeed .. ")")
            end
            
            if playerWrapper:HasCamera() then
                local camera = playerWrapper:GetCamera()
                Log("    Has Camera (zoom: " .. camera.zoom .. ")")
            end
        else
            LogWarning("  ✗ Player wrapper is invalid!")
        end
    end
    
    -- ================================================================
    -- TEST 6: Collision Constants
    -- ================================================================
    Log("\n[TEST 6] Collision Layer Constants:")
    Log("  NONE: " .. CollisionLayer.NONE)
    Log("  DEFAULT: " .. CollisionLayer.DEFAULT)
    Log("  PLAYER: " .. CollisionLayer.PLAYER)
    Log("  ENEMY: " .. CollisionLayer.ENEMY)
    Log("  WALL: " .. CollisionLayer.WALL)
    Log("  PROJECTILE: " .. CollisionLayer.PROJECTILE)
    Log("  PICKUP: " .. CollisionLayer.PICKUP)
    Log("  ALL: " .. CollisionLayer.ALL)
    
    Log("\n[TEST 6b] Collider Purpose Constants:")
    Log("  Physics: " .. ColliderPurpose.Physics)
    Log("  Environment: " .. ColliderPurpose.Environment)
    Log("  Trigger: " .. ColliderPurpose.Trigger)
    
    -- ================================================================
    -- TEST 7: Input Key Constants
    -- ================================================================
    Log("\n[TEST 7] Input Key Constants:")
    Log("  KEY_W: " .. KEY_W)
    Log("  KEY_A: " .. KEY_A)
    Log("  KEY_S: " .. KEY_S)
    Log("  KEY_D: " .. KEY_D)
    Log("  KEY_SPACE: " .. KEY_SPACE)
    Log("  KEY_SHIFT: " .. KEY_SHIFT)
    Log("  KEY_CTRL: " .. KEY_CTRL)
    Log("  KEY_E: " .. KEY_E)
    Log("  MOUSE_LEFT: " .. MOUSE_LEFT)
    Log("  MOUSE_RIGHT: " .. MOUSE_RIGHT)
    Log("  MOUSE_MIDDLE: " .. MOUSE_MIDDLE)
    
    -- ================================================================
    -- TEST 8: Vec2 Operations
    -- ================================================================
    -- Log("\n[TEST 8] Vec2 Math Operations:")
    -- local v1 = Vec2(10, 20)
    -- local v2 = Vec2(5, 3)
    -- 
    -- local vAdd = v1 + v2
    -- Log("  (10, 20) + (5, 3) = (" .. vAdd.x .. ", " .. vAdd.y .. ")")
    -- 
    -- local vSub = v1 - v2
    -- Log("  (10, 20) - (5, 3) = (" .. vSub.x .. ", " .. vSub.y .. ")")
    -- 
    -- local vMul = v1 * 2
    -- Log("  (10, 20) * 2 = (" .. vMul.x .. ", " .. vMul.y .. ")")
    
    Log("\n========================================")
    Log("START() COMPLETE - Press keys to test!")
    Log("========================================")
    Log("Controls:")
    Log("  E - Toggle collider debug visualization")
    Log("  WASD - Test movement input")
    Log("  SPACE - Move up")
    Log("  1/2/3 - Change collider purpose")
    Log("  T - Toggle collider active state")
    Log("  A - Add new collider shape")
    Log("  C - Clear collider shapes")
    Log("  I - Print detailed runtime info")
    Log("  M - Test collider modification")
end

function Update(dt)
    testCounter = testCounter + 1
    
    -- ================================================================
    -- TEST 9: Input System
    -- ================================================================
    
    -- Toggle collider debug visualization
    if KeyPressed(KEY_E) then
        if HasCollider() then
            local collider = GetCollider()
            collider.showBBox = not collider.showBBox
            Log("[INPUT] Collider debug: " .. tostring(collider.showBBox))
        end
    end
    
    -- Movement test (continuous input)
    if HasTransform() and HasRigidBody() then
        local rb = GetRigidBody()
        local moveVec = Vec2(0, 0)
        
        if KeyDown(KEY_I) then
            moveVec.y = moveVec.y + 1
            if not hasLoggedMovement then
                Log("[INPUT] I is held down")
                hasLoggedMovement = true
            end
        end
        if KeyDown(KEY_K) then moveVec.y = moveVec.y - 1 end
        if KeyDown(KEY_J) then moveVec.x = moveVec.x - 1 end
        if KeyDown(KEY_L) then moveVec.x = moveVec.x + 1 end
        
        -- Apply movement
        if moveVec.x ~= 0 or moveVec.y ~= 0 then
            local targetAccel = moveVec * speed
            -- Smooth interpolation like player
            currentAccel = currentAccel + (targetAccel - currentAccel) * accelSmoothFactor * dt
            rb.acceleration = currentAccel
        else
            -- Smooth to zero
            currentAccel = currentAccel + (Vec2(0, 0) - currentAccel) * accelSmoothFactor * dt
            rb.acceleration = currentAccel
            hasLoggedMovement = false
        end
    end
    
    -- Jump/move up test
    if KeyPressed(KEY_SPACE) then
        if HasTransform() then
            local tf = GetTransform()
            tf.position.y = tf.position.y + moveDistance
            Log("[INPUT] Moved up by " .. moveDistance)
        end
    end
    
    -- Collider modification tests
    if HasCollider() then
        local collider = GetCollider()
        
        -- Change collider purpose
        if KeyPressed(KEY_1) then
            local shape = collider.shapes[1]
            if shape then
                shape.purpose = ColliderPurpose.Physics
                Log("[INPUT] Set collider to Physics")
            end
        elseif KeyPressed(KEY_2) then
            local shape = collider.shapes[1]
            if shape then
                shape.purpose = ColliderPurpose.Environment
                Log("[INPUT] Set collider to Environment")
            end
        elseif KeyPressed(KEY_3) then
            local shape = collider.shapes[1]
            if shape then
                shape.purpose = ColliderPurpose.Trigger
                Log("[INPUT] Set collider to Trigger")
            end
        end
        
        -- Toggle active state
        if KeyPressed(KEY_T) then
            local shape = collider.shapes[1]
            if shape then
                shape.isActive = not shape.isActive
                Log("[INPUT] Collider active: " .. tostring(shape.isActive))
            end
        end
        
        -- Add new shape
        -- if KeyPressed(KEY_A) then
        --     local newShape = ColliderShape()
        --     newShape.size = Vec2(1.0, 1.0)
        --     newShape.offset = Vec2(2.0, 0.0)
        --     newShape.purpose = ColliderPurpose.Physics
        --     newShape.layer = CollisionLayer.DEFAULT
        --     newShape.colliderMask = CollisionLayer.ALL
        --     newShape.isActive = true
        --     newShape.autoFitToSprite = false
        --     
        --     collider.shapes:push_back(newShape)
        --     Log("[INPUT] Added new shape. Total: " .. collider.shapes:size())
        -- end
        
        -- Clear shapes
        if KeyPressed(KEY_C) then
            collider.shapes:clear()
            Log("[INPUT] Cleared all shapes")
        end
        
        -- Modify shape properties
        if KeyPressed(KEY_M) then
            local shape = collider.shapes[1]
            if shape then
                shape.size = Vec2(shape.size.x + 0.5, shape.size.y + 0.5)
                Log("[INPUT] Increased shape size to (" .. shape.size.x .. ", " .. shape.size.y .. ")")
            end
        end
    end
    
    -- Print detailed info
    if KeyPressed(KEY_I) then
        Log("\n========================================")
        Log("RUNTIME INFO (Frame " .. testCounter .. ")")
        Log("========================================")
        Log("Delta Time: " .. dt)
        Log("ExposedVars:")
        Log("  speed: " .. speed)
        Log("  name: " .. name)
        Log("  testCounter: " .. testCounter)
        
        if HasTransform() then
            local tf = GetTransform()
            Log("Current Position: (" .. tf.position.x .. ", " .. tf.position.y .. ")")
            Log("Current Rotation: (" .. tf.rotation.x .. ", " .. tf.rotation.y .. ")")
        end
        
        if HasRigidBody() then
            local rb = GetRigidBody()
            Log("Current Velocity: (" .. rb.velocity.x .. ", " .. rb.velocity.y .. ")")
            Log("Current Acceleration: (" .. rb.acceleration.x .. ", " .. rb.acceleration.y .. ")")
        end
        
        if HasCollider() then
            local collider = GetCollider()
            Log("Collider Shapes: " .. collider.shapes:size())
            Log("Collider Bounds: " .. collider.bounds:size())
        end
        
        if playerEntity ~= -1 then
            local playerTf = GetTransformFrom(playerEntity)
            if playerTf then
                Log("Player Position: (" .. playerTf.position.x .. ", " .. playerTf.position.y .. ")")
            end
        end
        
        Log("========================================\n")
    end
    
    -- Mouse test
    if MouseButtonPressed(MOUSE_LEFT) then
        local mousePos = GetMousePosition()
        Log("[INPUT] Left click at: (" .. mousePos.x .. ", " .. mousePos.y .. ")")
    end
    
    if MouseButtonDown(MOUSE_RIGHT) then
        if testCounter % 60 == 0 then  -- Log once per second
            local mousePos = GetMousePosition()
            Log("[INPUT] Right button held at: (" .. mousePos.x .. ", " .. mousePos.y .. ")")
        end
    end
    
    if MouseButtonReleased(MOUSE_MIDDLE) then
        Log("[INPUT] Middle mouse button released")
    end
end

function OnCollisionEnter(otherEntity)
    Log("\n[COLLISION ENTER] " .. name .. " (ID:" .. EntityID .. ") collided with entity " .. otherEntity)
    
    -- Use GetEntity wrapper
    local otherWrapper = GetEntity(otherEntity)
    if otherWrapper.isValid then
        Log("  Other entity ID: " .. otherWrapper.id)
        
        -- Check what components it has
        if otherWrapper:HasPlayer() then
            Log("  → Hit PLAYER")
        elseif otherWrapper:HasEnemy() then
            Log("  → Hit ENEMY")
        end
        
        -- Get transform
        if otherWrapper:HasTransform() then
            local otherTf = otherWrapper:GetTransform()
            Log("  Other position: (" .. otherTf.position.x .. ", " .. otherTf.position.y .. ")")
        end
        
        -- Get collider info
        if otherWrapper:HasCollider() then
            local otherCollider = otherWrapper:GetCollider()
            Log("  Other collider layer: " .. otherCollider.defaultLayer)
            if otherCollider.shapes:size() > 0 then
                local shape = otherCollider.shapes[1]
                Log("  Other shape purpose: " .. shape.purpose)
            end
        end
    end
end

function OnCollision(otherEntity)
    -- Only log occasionally to avoid spam
    if testCounter % 60 == 0 then  -- Once per second at 60fps
        Log("[COLLISION STAY] Still colliding with " .. otherEntity)
    end
end

function OnCollisionExit(otherEntity)
    Log("[COLLISION EXIT] " .. name .. " stopped colliding with " .. otherEntity)
end

function OnTriggerEnter(otherEntity)
    Log("[TRIGGER ENTER] " .. name .. " triggered by entity " .. otherEntity)
    
    if HasCollider() then
        local collider = GetCollider()
        local layer = collider:GetEffectiveLayer(0)
        local mask = collider:GetEffectiveMask(0)
        Log("  My layer: " .. layer .. ", My mask: " .. mask)
    end
    
    -- Check what triggered us
    local other = GetEntity(otherEntity)
    if other.isValid and other:HasPlayer() then
        Log("  → Triggered by PLAYER")
    end
end

function OnTriggerExit(otherEntity)
    Log("[TRIGGER EXIT] " .. name .. " trigger ended with " .. otherEntity)
end

function OnEnable()
    Log("[LIFECYCLE] Script ENABLED on entity " .. EntityID .. " (" .. name .. ")")
end

function OnDisable()
    Log("[LIFECYCLE] Script DISABLED on entity " .. EntityID .. " (" .. name .. ")")
end

function OnDestroy()
    Log("[LIFECYCLE] Script DESTROYED on entity " .. EntityID .. " (" .. name .. ")")
end