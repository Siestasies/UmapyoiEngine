# Lua Scripting API Documentation

Complete reference for the Lua scripting system in Uma Engine.

---

## Table of Contents

1. [Core Concepts](#core-concepts)
2. [Component Access](#component-access)
3. [Entity Management](#entity-management)
4. [Entity Queries](#entity-queries)
5. [Input System](#input-system)
6. [Collision System](#collision-system)
7. [Audio System](#audio-system)
8. [Utility Functions](#utility-functions)
9. [Script Lifecycle](#script-lifecycle)
10. [Examples](#examples)

---

## Core-Concepts

### Script Structure

Every Lua script should follow this basic structure:

```lua
-- Optional: Expose variables to the editor
ExposedVars = {
    speed = 10.0,
    health = 100,
    enabled = true,
    name = "Player"
}

-- Called once when script is initialized
function Start()
    -- Initialization code
end

-- Called every frame
function Update(dt)
    -- Per-frame logic
end

-- Called when entity is destroyed
function OnDestroy()
    -- Cleanup code
end
```

### Special Variables

- **`EntityID`**: The ID of the entity this script is attached to (automatically set)
- **`ExposedVars`**: Table of variables exposed to the editor for easy tweaking

---

## Component-Access

### Getting Components (Current Entity)

Access components attached to the same entity as your script:

#### `GetTransform()`
```lua
local transform = GetTransform()
if transform then
    transform.position.x = 100
    transform.rotation = 45
    transform.scale = Vec2(2, 2)
end
```

#### `GetRigidBody()`
```lua
local rb = GetRigidBody()
if rb then
    rb.velocity = Vec2(10, 0)
    rb.acceleration = Vec2(0, -9.8)
    rb.accel_strength = 5.0
    rb.fric_coeff = 0.5
end
```

#### `GetSprite()`
```lua
local sprite = GetSprite()
if sprite then
    sprite.textureName = "player.png"
    sprite.renderLayer = 1
    sprite.flipX = true
    sprite.flipY = false
    sprite.autoFlip = true
end
```

#### `GetCollider()`
```lua
local collider = GetCollider()
if collider then
    collider.showBBox = true
    local shape = collider:GetPrimaryShape()
    shape.isActive = true
end
```

#### `GetPlayer()`
```lua
local player = GetPlayer()
if player then
    player.mSpeed = 200
    player.mHealth = 100
    player.mMaxHealth = 100
    player.mHealthRegenRate = 5
    player.mDashSpeed = 400
    player.mDashCD = 1.0
    player.mAttackDamage = 25
    player.mAttackSpeed = 1.5
    player.mAttackRange = 50
    player.mDefense = 10
    player.mMana = 100
    player.mMaxMana = 100
    player.mManaRegenRate = 10
end
```

#### `GetEnemy()`
```lua
local enemy = GetEnemy()
if enemy then
    enemy.mSpeed = 150
    enemy.mHealth = 100
    enemy.mMaxHealth = 100
    enemy.mHealthRegenRate = 2
    enemy.mAttackDamage = 15
    enemy.mAttackSpeed = 1.0
    enemy.mAttackRange = 30
    enemy.mDefense = 5
end
```

#### `GetCamera()`
```lua
local camera = GetCamera()
if camera then
    camera.zoom = 1.5
    camera.followPlayer = true
end
```

#### `GetPathFinding()`
```lua
local pathfinding = GetPathFinding()
if pathfinding then
    pathfinding.goal = Vec2(100, 200)
    if pathfinding.reachedGoal then
        Log("Reached destination!")
    end
end
```

#### `GetProjectile()`
```lua
local projectile = GetProjectile()
if projectile then
    projectile.mDamage = 25
    projectile.mSpeed = 400
    projectile.mLifeTime = 3.0
    projectile.mFadeOVerTime = true
end
```

### Checking for Components (Current Entity)

#### `HasTransform()`, `HasRigidBody()`, etc.
```lua
if HasRigidBody() then
    local rb = GetRigidBody()
    -- Safe to use rb
end

if HasPathFinding() then
    local pf = GetPathFinding()
    -- Safe to use pathfinding
end

if HasProjectile() then
    local proj = GetProjectile()
    -- Safe to use projectile
end
```

---

## Entity-Management

### Creating and Destroying Entities

#### `CreateEntity()`
Creates a new entity and returns its ID.

```lua
local newEntity = CreateEntity()
Log("Created entity: " .. tostring(newEntity))
```

#### `DestroyEntity(entity)`
Destroys the specified entity.

```lua
DestroyEntity(targetEntity)
```

#### `DestroyWithChildren(entity)`
Destroys an entity and all its children.

```lua
DestroyWithChildren(parentEntity)
```

### Parent-Child Relationships

#### `SetParent(child, parent)`
Sets a parent-child relationship between entities.

```lua
SetParent(childEntity, parentEntity)
```

#### `RemoveParent(child)`
Removes the parent from an entity.

```lua
RemoveParent(childEntity)
```

#### `GetParent(entity)`
Returns the parent entity ID, or -1 if no parent exists.

```lua
local parentId = GetParent(EntityID)
if parentId ~= -1 then
    Log("Has parent: " .. tostring(parentId))
end
```

#### `HasParent(entity)`
Checks if an entity has a parent.

```lua
if HasParent(EntityID) then
    Log("This entity has a parent")
end
```

#### `GetChildren(entity)`
Returns an array of all child entity IDs.

```lua
local children = GetChildren(EntityID)
for i, childId in ipairs(children) do
    Log("Child " .. i .. ": " .. tostring(childId))
end
```

### Entity Activation

#### `SetActiveEntity(entity, isActive)`
Sets whether an entity is active or inactive.

```lua
-- Deactivate an entity
SetActiveEntity(enemyId, false)

-- Reactivate an entity
SetActiveEntity(enemyId, true)
```

---

## Entity-Queries

### Finding Entities

#### `FindEntitiesWithComponent(componentName)`
Returns an array of all entity IDs that have the specified component.

```lua
local enemies = FindEntitiesWithComponent("Enemy")
for i, enemyId in ipairs(enemies) do
    Log("Found enemy: " .. tostring(enemyId))
end
```

**Available component names:**
- `"Transform"`
- `"RigidBody"`
- `"Sprite"`
- `"Collider"`
- `"Player"`
- `"Enemy"`
- `"Camera"`
- `"PathFinding"`
- `"Projectile"`

#### `FindEntityWithComponent(componentName)`
Returns the first entity ID with the specified component, or -1 if not found.

```lua
local playerId = FindEntityWithComponent("Player")
if playerId ~= -1 then
    Log("Found player: " .. tostring(playerId))
end
```

#### `GetEntityCount()`
Returns the total number of active entities.

```lua
local count = GetEntityCount()
Log("Active entities: " .. tostring(count))
```

#### `IsEntityValid(entity)`
Checks if an entity ID is valid and active.

```lua
if IsEntityValid(targetEntity) then
    -- Safe to access
end
```

### Accessing Other Entities

#### Method 1: Entity Wrapper (Object-Oriented)

```lua
local player = GetEntity(playerId)
if player.isValid then
    local playerTransform = player:GetTransform()
    if playerTransform then
        playerTransform.position.x = 100
    end
    
    if player:HasRigidBody() then
        local rb = player:GetRigidBody()
        rb.velocity = Vec2(10, 0)
    end
    
    if player:HasPathFinding() then
        local pf = player:GetPathFinding()
        pf.goal = Vec2(200, 200)
    end
end
```

**Available wrapper methods:**
- `entity:GetTransform()` / `entity:HasTransform()`
- `entity:GetRigidBody()` / `entity:HasRigidBody()`
- `entity:GetSprite()` / `entity:HasSprite()`
- `entity:GetCollider()` / `entity:HasCollider()`
- `entity:GetPlayer()` / `entity:HasPlayer()`
- `entity:GetEnemy()` / `entity:HasEnemy()`
- `entity:GetCamera()` / `entity:HasCamera()`
- `entity:GetPathFinding()` / `entity:HasPathFinding()`
- `entity:GetProjectile()` / `entity:HasProjectile()`

#### Method 2: Direct Functions (Functional)

```lua
local transform = GetTransformFrom(targetEntity)
if transform then
    transform.position.y = 200
end

if HasRigidBodyOn(targetEntity) then
    local rb = GetRigidBodyFrom(targetEntity)
    rb.velocity = Vec2(0, 0)
end

if HasPathFindingOn(targetEntity) then
    local pf = GetPathFindingFrom(targetEntity)
    pf.goal = Vec2(300, 300)
end
```

**Available cross-entity functions:**
- `GetTransformFrom(entity)` / `HasTransformOn(entity)`
- `GetRigidBodyFrom(entity)` / `HasRigidBodyOn(entity)`
- `GetSpriteFrom(entity)` / `HasSpriteOn(entity)`
- `GetColliderFrom(entity)` / `HasColliderOn(entity)`
- `GetPlayerFrom(entity)` / `HasPlayerOn(entity)`
- `GetEnemyFrom(entity)` / `HasEnemyOn(entity)`
- `GetCameraFrom(entity)` / `HasCameraOn(entity)`
- `GetPathFindingFrom(entity)` / `HasPathFindingOn(entity)`
- `GetProjectileFrom(entity)` / `HasProjectileOn(entity)`

---

## Input-System

### Keyboard Input

#### `KeyDown(key)`
Returns true while the key is held down.

```lua
if KeyDown(KEY_W) then
    -- Move up while W is held
end
```

#### `KeyPressed(key)`
Returns true only on the frame the key is first pressed.

```lua
if KeyPressed(KEY_SPACE) then
    -- Jump once per press
end
```

#### `KeyReleased(key)`
Returns true only on the frame the key is released.

```lua
if KeyReleased(KEY_SHIFT) then
    -- Stop sprinting
end
```

### Mouse Input

#### `MouseButtonDown(button)`
Returns true while the mouse button is held down.

```lua
if MouseButtonDown(MOUSE_LEFT) then
    -- Firing continuously
end
```

#### `MouseButtonPressed(button)`
Returns true only on the frame the mouse button is first pressed.

```lua
if MouseButtonPressed(MOUSE_LEFT) then
    -- Fire once per click
end
```

#### `MouseButtonReleased(button)`
Returns true only on the frame the mouse button is released.

```lua
if MouseButtonReleased(MOUSE_LEFT) then
    -- Release action
end
```

#### `GetMousePosition()`
Returns the current mouse position as a Vec2.

```lua
local mousePos = GetMousePosition()
Log("Mouse: " .. tostring(mousePos.x) .. ", " .. tostring(mousePos.y))
```

### Input Constants

**Keyboard Keys:**
- `KEY_A` through `KEY_Z` - All alphabet keys
- `KEY_0` through `KEY_9` - Number keys
- `KEY_F1` through `KEY_F12` - Function keys
- `KEY_W`, `KEY_A`, `KEY_S`, `KEY_D` - WASD keys
- `KEY_U` - U key
- `KEY_SPACE` - Spacebar
- `KEY_SHIFT` - Left Shift
- `KEY_CTRL` - Left Control
- `KEY_E` - E key
- `KEY_ESCAPE` - Escape key
- `KEY_ENTER` - Enter/Return key
- `KEY_TAB` - Tab key
- `KEY_BACKSPACE` - Backspace key
- `KEY_DELETE` - Delete key

**Mouse Buttons:**
- `MOUSE_LEFT` - Left mouse button
- `MOUSE_RIGHT` - Right mouse button
- `MOUSE_MIDDLE` - Middle mouse button (scroll wheel click)

---

## Collision-System

### Collision Types

#### CollisionLayer
```lua
CollisionLayer.NONE        -- No collision layer
CollisionLayer.DEFAULT     -- Default collision layer
CollisionLayer.PLAYER      -- Player layer
CollisionLayer.ENEMY       -- Enemy layer
CollisionLayer.WALL        -- Wall/obstacle layer
CollisionLayer.PROJECTILE  -- Projectile layer
CollisionLayer.PICKUP      -- Pickup/collectible layer
CollisionLayer.ALL         -- All layers
```

#### ColliderPurpose
```lua
ColliderPurpose.Physics      -- Blocks movement (solid collision)
ColliderPurpose.Environment  -- Static obstacles
ColliderPurpose.Trigger      -- Passes through but triggers events
```

### Collision Events

#### `OnCollisionEnter(otherEntity)`
Called once when collision starts (Physics/Environment colliders).

```lua
function OnCollisionEnter(other)
    Log("Collision started with: " .. tostring(other))
    
    if HasEnemyOn(other) then
        Log("Hit an enemy!")
    end
end
```

#### `OnCollision(otherEntity)`
Called every frame during collision (Physics/Environment colliders).

```lua
function OnCollision(other)
    -- Handle continuous collision
    -- e.g., push back, take damage over time
end
```

#### `OnCollisionExit(otherEntity)`
Called once when collision ends (Physics/Environment colliders).

```lua
function OnCollisionExit(other)
    Log("Collision ended with: " .. tostring(other))
end
```

### Trigger Events

#### `OnTriggerEnter(otherEntity)`
Called once when entering a trigger collider.

```lua
function OnTriggerEnter(other)
    if HasPlayerOn(other) then
        Log("Player entered pickup zone")
        -- Collect item logic
    end
end
```

#### `OnTrigger(otherEntity)`
Called every frame while inside a trigger collider.

```lua
function OnTrigger(other)
    -- Handle continuous trigger overlap
    -- e.g., healing zone, speed boost zone
end
```

#### `OnTriggerExit(otherEntity)`
Called once when leaving a trigger collider.

```lua
function OnTriggerExit(other)
    Log("Left trigger zone")
    -- Remove effect
end
```

### Collider Manipulation

```lua
local collider = GetCollider()
if collider then
    -- Access primary shape
    local shape = collider:GetPrimaryShape()
    shape.size = Vec2(32, 32)
    shape.offset = Vec2(0, 0)
    shape.purpose = ColliderPurpose.Trigger
    shape.layer = CollisionLayer.PICKUP
    shape.colliderMask = CollisionLayer.PLAYER
    shape.isActive = true
    
    -- Get bounding box
    local bounds = collider:GetPrimaryBounds()
    Log("Bounds min: " .. tostring(bounds.min))
    Log("Bounds max: " .. tostring(bounds.max))
    
    -- Access all shapes (if multiple colliders)
    for i = 1, #collider.shapes do
        local s = collider.shapes[i]
        s.isActive = true
    end
    
    -- Show debug bounding box
    collider.showBBox = true
end
```

**Collider Properties:**
- `size` - Vec2 dimensions of the collider
- `offset` - Vec2 offset from entity position
- `purpose` - ColliderPurpose enum
- `layer` - CollisionLayer enum for this collider
- `colliderMask` - CollisionLayer enum for what it collides with
- `isActive` - Boolean to enable/disable collider
- `autoFitToSprite` - Boolean to auto-size to sprite

---

## Audio-System

### Sound Effects

#### `PlaySound(audioName, volume, loops)`
Plays a sound effect.

**Parameters:**
- `audioName` (string): The name you assigned in ResourceManager (e.g., "explosion", "jump")
- `volume` (float): Volume level from 0.0 to 1.0
- `loops` (int): Number of times to loop (0 = play once, -1 = loop forever)

```lua
-- Play jump sound once at 80% volume
PlaySound("jump", 0.8, 0)

-- Play laser sound at full volume
PlaySound("laser", 1.0, 0)

-- Loop footstep sound at 60% volume
PlaySound("footsteps", 0.6, -1)
```

#### `StopSound(audioName)`
Stops a playing sound effect.

```lua
-- Stop looping footsteps
StopSound("footsteps")

-- Stop explosion sound
StopSound("explosion")
```

### Music

#### `PlayMusic(audioName, volume, loops)`
Plays background music.

**Parameters:**
- `audioName` (string): The name you assigned in ResourceManager (e.g., "cave", "battle_theme")
- `volume` (float): Volume level from 0.0 to 1.0
- `loops` (int): Number of times to loop (0 = play once, -1 = loop forever)

```lua
-- Play cave music looping forever at 50% volume
PlayMusic("cave", 0.5, -1)

-- Play boss music at 70% volume
PlayMusic("boss_theme", 0.7, -1)

-- Play victory jingle once
PlayMusic("victory", 0.9, 0)
```

#### `StopMusic(audioName)`
Stops playing music.

```lua
-- Stop cave music
StopMusic("cave")

-- Stop all music by name
StopMusic("boss_theme")
```

### Spatial Audio (NEW)

#### `PlayEntitySound(entity, audioName, loop, volume)`
Plays a sound attached to a specific entity (follows entity position).

**Parameters:**
- `entity` (Entity): The entity to attach the sound to
- `audioName` (string): The audio resource name
- `loop` (bool): Whether the sound should loop
- `volume` (float): Volume level from 0.0 to 1.0

```lua
-- Play engine sound that follows the entity
PlayEntitySound(EntityID, "engine_loop", true, 0.7)

-- Play one-time sound at entity
PlayEntitySound(enemyId, "enemy_growl", false, 0.8)
```

#### `StopEntitySound(entity)`
Stops all sounds attached to an entity.

```lua
StopEntitySound(EntityID)
```

#### `StopEntitySoundByName(entity, soundName)`
Stops a specific sound by name attached to an entity.

```lua
StopEntitySoundByName(EntityID, "engine_loop")
```

#### `PlayOneShotAtEntity(entity, audioName, volume)`
Plays a one-time sound at an entity's position (doesn't follow entity).

```lua
PlayOneShotAtEntity(EntityID, "explosion", 1.0)
```

#### `PlayOneShotAtPosition(x, y, audioName, volume)`
Plays a one-time sound at a specific world position.

```lua
local pos = GetTransform().position
PlayOneShotAtPosition(pos.x, pos.y, "impact", 0.9)
```

> **Note:** Audio names are the identifiers you set when loading resources in the ResourceManager, NOT file paths or extensions. For example, if you loaded "Assets/Audio/explosion.wav" with the name "explosion", you would use `PlaySound("explosion", 1.0, 0)`.

---

## Utility-Functions

### Logging

#### `Log(message)`
Logs an info message to the console.

```lua
Log("Player health: " .. tostring(health))
Log("Game started")
```

#### `LogWarning(message)`
Logs a warning message to the console.

```lua
LogWarning("Low health!")
LogWarning("Missing component")
```

#### `LogError(message)`
Logs an error message to the console.

```lua
LogError("Critical failure!")
LogError("Cannot find player entity")
```

### Time

#### `GetDeltaTime()`
Returns the time elapsed since the last frame (in seconds).

```lua
function Update(dt)
    -- Both methods work - dt parameter or GetDeltaTime()
    local deltaTime = GetDeltaTime()
    
    -- Use for frame-rate independent movement
    local movement = speed * deltaTime
    
    -- Update timer
    timer = timer + deltaTime
end
```

### Animation (NEW)

#### `PlayAnimation(entity, animationName)`
Plays an animation on the specified entity.

```lua
-- Play walk animation on this entity
PlayAnimation(EntityID, "walk")

-- Play attack animation on an enemy
PlayAnimation(enemyId, "attack")
```

### Physics (NEW)

#### `AddForce(entity, position, direction, force, rotation)`
Applies a force to an entity with RigidBody.

**Parameters:**
- `entity` (Entity): Target entity
- `position` (Vec2): New position to set
- `direction` (Vec2): Direction vector (will be normalized)
- `force` (float): Force magnitude
- `rotation` (float): Rotation angle

```lua
-- Launch projectile
local dir = Vec2(1, 0.5)
AddForce(projectileId, Vec2(100, 100), dir, 500, 45)

-- Knockback effect
local knockbackDir = Vec2(-1, 0)
AddForce(enemyId, currentPos, knockbackDir, 300, 0)
```

### Pathfinding (NEW)

#### `SetPathFindingGoal(entity, x, y)`
Sets the goal position for an entity with PathFinding component.

```lua
-- Set enemy to pathfind to player position
local playerPos = GetTransformFrom(playerId).position
SetPathFindingGoal(enemyId, playerPos.x, playerPos.y)

-- Set goal to specific location
SetPathFindingGoal(EntityID, 500, 300)
```

### Prefab Spawning (NEW)

#### `SpawnPrefab(prefabName, position)`
Spawns a prefab at the specified position and returns the root entity ID.

**Parameters:**
- `prefabName` (string): Name of the prefab file (including .json extension)
- `position` (Vec2): World position to spawn at

**Returns:**
- `Entity`: The root entity ID of the spawned prefab, or -1 on failure

```lua
-- Spawn enemy prefab at position
local spawnPos = Vec2(200, 100)
local enemyId = SpawnPrefab("Enemy.json", spawnPos)

if enemyId ~= -1 then
    Log("Enemy spawned successfully: " .. tostring(enemyId))
else
    LogError("Failed to spawn enemy prefab")
end

-- Spawn projectile
local projectileId = SpawnPrefab("Fireball.json", GetTransform().position)
```

> **Note:** Prefab files must be located in the prefabs directory and include the `.json` extension in the filename.

### Scene Management (NEW)

#### `LoadScene(sceneName)`
Loads a different scene.

```lua
-- Load main menu
LoadScene("MainMenu")

-- Load next level
LoadScene("Level2")
```

#### `CloseApplication()`
Closes the application/game.

```lua
-- Quit game
if KeyPressed(KEY_ESCAPE) then
    CloseApplication()
end
```

---

## Script-Lifecycle

### Lifecycle Functions

Scripts can implement these optional functions. They will be called automatically by the engine at the appropriate times:

#### `Start()`
Called once when the script is first initialized, after all components are loaded.

**Use for:**
- Finding other entities
- Caching references
- Initial setup
- Subscribing to events

```lua
function Start()
    Log("Script started!")
    
    -- Cache player reference
    playerId = FindEntityWithComponent("Player")
    
    -- Store initial position
    initialPosition = GetTransform().position
    
    -- Play start sound
    PlaySound("spawn", 0.7, 0)
end
```

#### `Update(dt)`
Called every frame. `dt` is the delta time in seconds since the last frame.

**Use for:**
- Movement logic
- Input handling
- Game logic
- Animations
- Timers

```lua
function Update(dt)
    -- Frame-rate independent movement
    local transform = GetTransform()
    transform.position.x = transform.position.x + (speed * dt)
    
    -- Update timer
    timer = timer + dt
    
    -- Handle input
    if KeyDown(KEY_W) then
        -- Move up
    end
end
```

#### `OnEnable()`
Called when the script is enabled (either initially or after being disabled).

**Use for:**
- Re-initializing state
- Resuming behavior
- Re-subscribing to events

```lua
function OnEnable()
    Log("Script enabled")
    isActive = true
    PlayMusic("theme", 0.5, -1)
end
```

#### `OnDisable()`
Called when the script is disabled.

**Use for:**
- Pausing behavior
- Stopping sounds
- Cleaning up temporary state

```lua
function OnDisable()
    Log("Script disabled")
    isActive = false
    StopMusic("theme")
end
```

#### `OnDestroy()`
Called when the entity is destroyed. Use for cleanup.

**Use for:**
- Stopping looping sounds
- Cleaning up spawned entities
- Saving state
- Unsubscribing from events

```lua
function OnDestroy()
    Log("Cleaning up resources")
    
    -- Stop any looping sounds
    StopSound("engine_loop")
    
    -- Clean up spawned objects
    for i, spawnId in ipairs(spawnedObjects) do
        DestroyEntity(spawnId)
    end
end
```

#### `OnClicked()` (NEW)
Called when a UI button with this script is clicked.

**Use for:**
- Button click handlers
- UI interactions

```lua
function OnClicked()
    Log("Button clicked!")
    PlaySound("button_click", 0.8, 0)
    LoadScene("MainMenu")
end
```

### Execution Order

1. **Initialization Phase:**
   - Script file is loaded and executed
   - `ExposedVars` are discovered
   - Variables are synced to Lua
   - `Start()` is called

2. **Active Phase:**
   - `OnEnable()` is called (if becoming enabled)
   - `Update(dt)` is called every frame
   - Collision/Trigger callbacks are called when events occur
   - `OnDisable()` is called (if becoming disabled)

3. **Destruction Phase:**
   - `OnDestroy()` is called
   - Script is cleaned up

---

## Examples

### Example 1: Player Movement

```lua
ExposedVars = {
    speed = 200.0,
    jumpForce = 500.0,
    isGrounded = false
}

function Start()
    Log("Player controller initialized")
end

function Update(dt)
    local rb = GetRigidBody()
    if not rb then return end
    
    -- Horizontal movement
    local moveX = 0
    if KeyDown(KEY_A) then moveX = moveX - 1 end
    if KeyDown(KEY_D) then moveX = moveX + 1 end
    
    rb.velocity.x = moveX * speed
    
    -- Flip sprite based on direction
    local sprite = GetSprite()
    if sprite and moveX ~= 0 then
        sprite.flipX = (moveX < 0)
    end
    
    -- Jump
    if KeyPressed(KEY_SPACE) and isGrounded then
        rb.velocity.y = jumpForce
        PlaySound("jump", 0.7, 0)
        isGrounded = false
    end
end

function OnCollisionEnter(other)
    -- Check if landed on ground
    local otherCollider = GetColliderFrom(other)
    if otherCollider then
        local purpose = otherCollider:GetPrimaryShape().purpose
        if purpose == ColliderPurpose.Environment then
            isGrounded = true
        end
    end
end
```

### Example 2: Enemy AI with State Machine

```lua
ExposedVars = {
    detectionRange = 200.0,
    attackRange = 50.0,
    moveSpeed = 100.0,
    attackCooldown = 1.0,
    health = 100
}

local playerId = -1
local state = "idle"  -- idle, chase, attack
local attackTimer = 0

function Start()
    playerId = FindEntityWithComponent("Player")
    if playerId == -1 then
        LogWarning("No player found!")
    end
end

function Update(dt)
    if playerId == -1 or not IsEntityValid(playerId) then return end
    
    -- Update attack timer
    attackTimer = attackTimer - dt
    
    local myTransform = GetTransform()
    local playerTransform = GetTransformFrom(playerId)
    if not playerTransform then return end
    
    -- Calculate distance to player
    local dx = playerTransform.position.x - myTransform.position.x
    local dy = playerTransform.position.y - myTransform.position.y
    local distance = math.sqrt(dx * dx + dy * dy)
    
    -- State machine
    if distance < attackRange then
        state = "attack"
        HandleAttackState(dt)
    elseif distance < detectionRange then
        state = "chase"
        HandleChaseState(dt, dx, distance)
    else
        state = "idle"
        HandleIdleState(dt)
    end
end

function HandleChaseState(dt, dx, distance)
    local rb = GetRigidBody()
    if rb then
        local dirX = dx / distance
        rb.velocity.x = dirX * moveSpeed
        
        -- Flip sprite to face player
        local sprite = GetSprite()
        if sprite then
            sprite.flipX = (dirX < 0)
        end
    end
end

function HandleAttackState(dt)
    -- Stop moving
    local rb = GetRigidBody()
    if rb then
        rb.velocity.x = 0
    end
    
    -- Attack if cooldown ready
    if attackTimer <= 0 then
        Log("Enemy attacking!")
        PlaySound("enemy_attack", 0.8, 0)
        attackTimer = attackCooldown
    end
end

function HandleIdleState(dt)
    -- Stop moving
    local rb = GetRigidBody()
    if rb then
        rb.velocity.x = 0
    end
end

function OnCollisionEnter(other)
    -- Take damage from projectiles
    if HasColliderOn(other) then
        local collider = GetColliderFrom(other)
        if collider:GetEffectiveLayer() == CollisionLayer.PROJECTILE then
            health = health - 10
            PlaySound("enemy_hit", 0.7, 0)
            
            if health <= 0 then
                Log("Enemy died!")
                PlaySound("enemy_death", 0.8, 0)
                DestroyEntity(EntityID)
            end
        end
    end
end
```

### Example 3: Collectible Pickup

```lua
ExposedVars = {
    points = 10,
    healAmount = 20,
    collectType = "coin",  -- coin, health, powerup
    collected = false
}

local rotationSpeed = 180  -- degrees per second
local bobSpeed = 2.0
local bobAmount = 10.0
local timer = 0
local startY = 0

function Start()
    local transform = GetTransform()
    startY = transform.position.y
    
    -- Make sure we're a trigger
    local collider = GetCollider()
    if collider then
        local shape = collider:GetPrimaryShape()
        shape.purpose = ColliderPurpose.Trigger
        shape.layer = CollisionLayer.PICKUP
        shape.colliderMask = CollisionLayer.PLAYER
    end
end

function Update(dt)
    if collected then return end
    
    timer = timer + dt
    local transform = GetTransform()
    
    -- Rotate
    transform.rotation = transform.rotation + (rotationSpeed * dt)
    
    -- Bob up and down
    transform.position.y = startY + math.sin(timer * bobSpeed) * bobAmount
end

function OnTriggerEnter(other)
    if collected then return end
    
    if HasPlayerOn(other) then
        collected = true
        
        Log("Player collected " .. collectType .. " worth " .. tostring(points) .. " points!")
        
        -- Play collection sound based on type
        if collectType == "coin" then
            PlaySound("coin_pickup", 0.8, 0)
        elseif collectType == "health" then
            PlaySound("heal", 0.7, 0)
        elseif collectType == "powerup" then
            PlaySound("powerup", 0.9, 0)
        end
        
        -- Destroy after short delay (could spawn particle effect here)
        DestroyEntity(EntityID)
    end
end
```

### Example 4: Smooth Camera Follow

```lua
ExposedVars = {
    smoothSpeed = 5.0,
    offset = Vec2(0, 50),
    lookAheadDistance = 100.0,
    boundaryEnabled = false,
    minX = -500,
    maxX = 500,
    minY = -300,
    maxY = 300
}

local targetId = -1
local currentVelocity = Vec2(0, 0)

function Start()
    targetId = FindEntityWithComponent("Player")
    
    if targetId == -1 then
        LogError("Camera: No player found to follow!")
    else
        Log("Camera following entity: " .. tostring(targetId))
    end
end

function Update(dt)
    if targetId == -1 or not IsEntityValid(targetId) then return end
    
    local myTransform = GetTransform()
    local targetTransform = GetTransformFrom(targetId)
    if not targetTransform then return end
    
    -- Get target velocity for look-ahead
    local targetRb = GetRigidBodyFrom(targetId)
    local lookAhead = Vec2(0, 0)
    
    if targetRb then
        lookAhead.x = targetRb.velocity.x * 0.2
        lookAhead.y = targetRb.velocity.y * 0.1
    end
    
    -- Calculate desired position with look-ahead
    local desiredPos = Vec2(
        targetTransform.position.x + offset.x + lookAhead.x,
        targetTransform.position.y + offset.y + lookAhead.y
    )
    
    -- Apply boundaries if enabled
    if boundaryEnabled then
        desiredPos.x = math.max(minX, math.min(maxX, desiredPos.x))
        desiredPos.y = math.max(minY, math.min(maxY, desiredPos.y))
    end
    
    -- Smooth follow
    local currentPos = myTransform.position
    myTransform.position.x = currentPos.x + (desiredPos.x - currentPos.x) * smoothSpeed * dt
    myTransform.position.y = currentPos.y + (desiredPos.y - currentPos.y) * smoothSpeed * dt
end
```

### Example 5: Enemy Spawner System

```lua
ExposedVars = {
    spawnInterval = 2.0,
    maxSpawns = 10,
    spawnRadius = 50.0,
    enemySpeed = 100.0,
    autoStart = true,
    enemyPrefab = "Enemy.json"
}

local timer = 0
local spawnCount = 0
local isActive = false
local spawnedEnemies = {}

function Start()
    Log("Spawner initialized")
    
    if autoStart then
        isActive = true
        PlaySound("spawner_active", 0.5, -1)
    end
end

function Update(dt)
    if not isActive or spawnCount >= maxSpawns then
        return
    end
    
    timer = timer + dt
    
    if timer >= spawnInterval then
        SpawnEnemy()
        timer = 0
    end
end

function SpawnEnemy()
    -- Get spawn position with random offset
    local myTransform = GetTransform()
    local angle = math.random() * math.pi * 2
    local distance = math.random() * spawnRadius
    
    local spawnPos = Vec2(
        myTransform.position.x + math.cos(angle) * distance,
        myTransform.position.y + math.sin(angle) * distance
    )
    
    -- Spawn enemy from prefab
    local enemyId = SpawnPrefab(enemyPrefab, spawnPos)
    
    if enemyId ~= -1 then
        table.insert(spawnedEnemies, enemyId)
        spawnCount = spawnCount + 1
        Log("Spawned enemy " .. tostring(spawnCount) .. "/" .. tostring(maxSpawns))
        PlaySound("enemy_spawn", 0.7, 0)
    else
        LogError("Failed to spawn enemy prefab")
    end
    
    -- Check if reached max spawns
    if spawnCount >= maxSpawns then
        isActive = false
        StopSound("spawner_active")
        Log("Spawner reached max capacity")
    end
end

function OnTriggerEnter(other)
    -- Activate spawner when player enters
    if not isActive and HasPlayerOn(other) then
        isActive = true
        Log("Spawner activated by player!")
        PlaySound("spawner_active", 0.5, -1)
    end
end

function OnDestroy()
    -- Clean up spawned enemies
    for i, enemyId in ipairs(spawnedEnemies) do
        if IsEntityValid(enemyId) then
            DestroyEntity(enemyId)
        end
    end
    
    StopSound("spawner_active")
    Log("Spawner destroyed, cleaned up " .. tostring(#spawnedEnemies) .. " enemies")
end
```

### Example 6: Projectile with Lifetime

```lua
ExposedVars = {
    speed = 400.0,
    lifetime = 3.0,
    damage = 25,
    direction = Vec2(1, 0)
}

local timeAlive = 0
local hasHit = false

function Start()
    -- Normalize direction
    local length = math.sqrt(direction.x * direction.x + direction.y * direction.y)
    if length > 0 then
        direction.x = direction.x / length
        direction.y = direction.y / length
    end
    
    -- Set initial velocity
    local rb = GetRigidBody()
    if rb then
        rb.velocity = Vec2(direction.x * speed, direction.y * speed)
    end
    
    -- Set rotation to face direction
    local transform = GetTransform()
    transform.rotation = math.atan2(direction.y, direction.x) * (180 / math.pi)
    
    PlaySound("projectile_fire", 0.6, 0)
end

function Update(dt)
    timeAlive = timeAlive + dt
    
    -- Destroy after lifetime expires
    if timeAlive >= lifetime then
        Log("Projectile expired")
        DestroyEntity(EntityID)
    end
end

function OnCollisionEnter(other)
    if hasHit then return end
    
    -- Don't hit the shooter (could add shooter ID check here)
    
    -- Hit enemy
    if HasEnemyOn(other) then
        Log("Projectile hit enemy for " .. tostring(damage) .. " damage")
        PlaySound("projectile_hit", 0.7, 0)
        hasHit = true
        DestroyEntity(EntityID)
    end
    
    -- Hit wall
    local collider = GetColliderFrom(other)
    if collider then
        local purpose = collider:GetPrimaryShape().purpose
        if purpose == ColliderPurpose.Environment then
            Log("Projectile hit wall")
            PlaySound("projectile_impact", 0.6, 0)
            DestroyEntity(EntityID)
        end
    end
end
```

### Example 7: UI Button Handler (NEW)

```lua
ExposedVars = {
    targetScene = "MainMenu",
    buttonText = "Start Game"
}

function Start()
    Log("Button initialized: " .. buttonText)
end

function OnClicked()
    Log("Button clicked: " .. buttonText)
    PlaySound("button_click", 0.8, 0)
    
    -- Load target scene
    LoadScene(targetScene)
end
```

### Example 8: Pathfinding Enemy (NEW)

```lua
ExposedVars = {
    detectionRange = 300.0,
    updateInterval = 0.5
}

local playerId = -1
local updateTimer = 0

function Start()
    playerId = FindEntityWithComponent("Player")
    
    if playerId == -1 then
        LogWarning("No player found for pathfinding!")
    end
end

function Update(dt)
    if playerId == -1 or not IsEntityValid(playerId) then return end
    
    updateTimer = updateTimer + dt
    
    -- Update pathfinding goal periodically
    if updateTimer >= updateInterval then
        updateTimer = 0
        
        local myPos = GetTransform().position
        local playerPos = GetTransformFrom(playerId).position
        
        -- Calculate distance
        local dx = playerPos.x - myPos.x
        local dy = playerPos.y - myPos.y
        local distance = math.sqrt(dx * dx + dy * dy)
        
        -- Set pathfinding goal if in range
        if distance < detectionRange then
            SetPathFindingGoal(EntityID, playerPos.x, playerPos.y)
        end
    end
    
    -- Check if reached goal
    local pf = GetPathFinding()
    if pf and pf.reachedGoal then
        Log("Reached player!")
    end
end
```

---

## Vec2 Helper

### Constructor

```lua
local v = Vec2()           -- Creates Vec2(0, 0)
local v = Vec2(10, 20)     -- Creates Vec2(10, 20)
```

### Properties

```lua
local pos = Vec2(100, 200)
pos.x = 150   -- Set x component
pos.y = 250   -- Set y component

Log("X: " .. tostring(pos.x))  -- Access x component
Log("Y: " .. tostring(pos.y))  -- Access y component
```

### Operations

```lua
local a = Vec2(10, 20)
local b = Vec2(5, 10)

-- Addition
local sum = a + b           -- Vec2(15, 30)

-- Subtraction
local diff = a - b          -- Vec2(5, 10)

-- Scalar multiplication
local scaled = a * 2        -- Vec2(20, 40)
local scaled2 = 2 * a       -- Vec2(20, 40) - both orders work

-- Scalar division
local divided = a / 2       -- Vec2(5, 10)

-- String representation
Log(tostring(a))            -- "Vec2(10, 20)"
```

### Common Vec2 Patterns

```lua
-- Distance between two points
function Distance(a, b)
    local dx = b.x - a.x
    local dy = b.y - a.y
    return math.sqrt(dx * dx + dy * dy)
end

-- Normalize a vector
function Normalize(v)
    local length = math.sqrt(v.x * v.x + v.y * v.y)
    if length > 0 then
        return Vec2(v.x / length, v.y / length)
    end
    return Vec2(0, 0)
end

-- Direction from A to B
function Direction(from, to)
    local dir = to - from
    return Normalize(dir)
end

-- Dot product
function Dot(a, b)
    return a.x * b.x + a.y * b.y
end

-- Lerp (linear interpolation)
function Lerp(a, b, t)
    return Vec2(
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t
    )
end
```

---

## Best Practices

### Performance

1. **Cache entity references** in `Start()` when possible
   ```lua
   local playerId = -1
   
   function Start()
       playerId = FindEntityWithComponent("Player")  -- Cache once
   end
   
   function Update(dt)
       if playerId ~= -1 then
           -- Use cached reference
       end
   end
   ```

2. **Check validity before accessing** cross-entity data
   ```lua
   if IsEntityValid(targetId) then
       local transform = GetTransformFrom(targetId)
       if transform then
           -- Safe to use
       end
   end
   ```

3. **Avoid repeated lookups** in Update()
   ```lua
   -- Bad
   function Update(dt)
       local transform = GetTransform()  -- Called every frame
       transform.position.x = transform.position.x + 1
   end
   
   -- Better for multiple accesses
   function Update(dt)
       local transform = GetTransform()
       local pos = transform.position
       pos.x = pos.x + 1
       pos.y = pos.y + 1
   end
   ```

### Safety

1. **Always check for nil** when accessing components
   ```lua
   local rb = GetRigidBody()
   if rb then
       rb.velocity = Vec2(10, 0)
   end
   ```

2. **Use `HasComponent()` before `GetComponent()`** for extra safety
   ```lua
   if HasRigidBody() then
       local rb = GetRigidBody()
       -- Guaranteed to be non-nil
   end
   ```

3. **Validate entity IDs** from queries
   ```lua
   local playerId = FindEntityWithComponent("Player")
   if playerId ~= -1 and IsEntityValid(playerId) then
       -- Safe to use
   end
   ```

### Frame-Rate Independence

1. **Multiply movement by delta time**
   ```lua
   function Update(dt)
       local transform = GetTransform()
       transform.position.x = transform.position.x + (speed * dt)
   end
   ```

2. **Use timers with delta time**
   ```lua
   local cooldownTimer = 0
   
   function Update(dt)
       cooldownTimer = cooldownTimer - dt
       
       if cooldownTimer <= 0 then
           -- Ready to fire
           cooldownTimer = cooldownDuration
       end
   end
   ```

### Organization

1. **Use ExposedVars** for designer-tweakable values
   ```lua
   ExposedVars = {
       speed = 200.0,      -- Easy to adjust
       jumpForce = 500.0,
       maxHealth = 100
   }
   ```

2. **Organize code with local helper functions**
   ```lua
   local function CalculateDistance(a, b)
       local dx = b.x - a.x
       local dy = b.y - a.y
       return math.sqrt(dx * dx + dy * dy)
   end
   
   function Update(dt)
       local dist = CalculateDistance(pos1, pos2)
   end
   ```

3. **Use meaningful variable names**
   ```lua
   -- Bad
   local e = FindEntityWithComponent("Enemy")
   
   -- Good
   local enemyId = FindEntityWithComponent("Enemy")
   ```

### Cleanup

1. **Stop looping sounds** in `OnDestroy()`
   ```lua
   function OnDestroy()
       StopSound("engine_loop")
       StopMusic("boss_theme")
       StopEntitySound(EntityID)
   end
   ```

2. **Destroy spawned entities** when parent is destroyed
   ```lua
   local spawnedObjects = {}
   
   function OnDestroy()
       for i, objId in ipairs(spawnedObjects) do
           if IsEntityValid(objId) then
               DestroyEntity(objId)
           end
       end
   end
   ```

3. **Clean up resources** properly
   ```lua
   function OnDestroy()
       -- Stop sounds
       StopSound("loop")
       StopEntitySound(EntityID)
       
       -- Clear references
       playerId = -1
       spawnedObjects = {}
       
       Log("Cleanup complete")
   end
   ```

---

## Common Patterns

### Singleton Pattern (Finding Single Entity)

```lua
local playerId = -1

function Start()
    playerId = FindEntityWithComponent("Player")
    
    if playerId == -1 then
        LogError("Player not found!")
    end
end

function Update(dt)
    if playerId == -1 or not IsEntityValid(playerId) then
        return
    end
    
    -- Use player reference
end
```

### Tracking Multiple Entities

```lua
local enemies = {}

function Start()
    RefreshEnemyList()
end

function RefreshEnemyList()
    enemies = FindEntitiesWithComponent("Enemy")
    Log("Found " .. tostring(#enemies) .. " enemies")
end

function Update(dt)
    -- Process each enemy
    for i, enemyId in ipairs(enemies) do
        if IsEntityValid(enemyId) then
            -- Do something with enemy
        end
    end
end
```

### State Machine Pattern

```lua
local state = "idle"
local states = {
    idle = HandleIdleState,
    chase = HandleChaseState,
    attack = HandleAttackState
}

function Update(dt)
    if states[state] then
        states[state](dt)
    end
end

function HandleIdleState(dt)
    -- Idle logic
end

function HandleChaseState(dt)
    -- Chase logic
end

function HandleAttackState(dt)
    -- Attack logic
end
```

### Timer Pattern

```lua
local timer = 0
local interval = 2.0

function Update(dt)
    timer = timer + dt
    
    if timer >= interval then
        timer = 0  -- or timer = timer - interval for precise timing
        OnTimerExpired()
    end
end

function OnTimerExpired()
    -- Do something periodically
end
```

---

## Troubleshooting

### Common Issues

1. **"Attempt to index a nil value"**
   - Always check if component exists before accessing
   - Use `if component then ... end`

2. **Movement is too fast/slow**
   - Make sure to multiply by `dt` for frame-rate independence
   - Check ExposedVars values

3. **Entity not found**
   - Verify component name spelling (case-sensitive)
   - Check if entity exists in the scene
   - Use `IsEntityValid()` to confirm

4. **Sounds not playing**
   - Verify audio name matches ResourceManager registration
   - Check volume is not 0
   - Ensure audio file was loaded successfully

5. **Collision not detecting**
   - Verify both entities have Collider components
   - Check collision layers and masks match
   - Ensure colliders are active (`isActive = true`)

6. **Prefab not spawning**
   - Verify prefab file exists in prefabs directory
   - Include `.json` extension in filename
   - Check console for error messages

### Debugging Tips

```lua
-- Log everything in Start() to verify initialization
function Start()
    Log("=== Script Starting ===")
    Log("Entity ID: " .. tostring(EntityID))
    
    if HasTransform() then
        local t = GetTransform()
        Log("Position: " .. tostring(t.position))
    end
    
    Log("=== Start Complete ===")
end

-- Add frame counters to track Update calls
local frameCount = 0

function Update(dt)
    frameCount = frameCount + 1
    
    if frameCount % 60 == 0 then  -- Log every 60 frames
        Log("Frame: " .. tostring(frameCount))
    end
end

-- Use LogWarning for important events
function OnCollisionEnter(other)
    LogWarning("Collision with entity: " .. tostring(other))
end
```

---

## Advanced Topics

### Require System

You can split code into modules using Lua's `require` system. The search paths are already configured:

```lua
-- In Assets/Scripts/Utils.lua
local Utils = {}

function Utils.Distance(a, b)
    local dx = b.x - a.x
    local dy = b.y - a.y
    return math.sqrt(dx * dx + dy * dy)
end

return Utils

-- In your script
local Utils = require("Utils")

function Update(dt)
    local dist = Utils.Distance(pos1, pos2)
end
```

### State Files

Place state machine definitions in separate files:

```lua
-- In Assets/Scripts/States/EnemyStates.lua
local States = {}

function States.Idle(entity, dt)
    -- Idle behavior
end

function States.Chase(entity, dt)
    -- Chase behavior
end

return States

-- In your enemy script
local EnemyStates = require("EnemyStates")

function Update(dt)
    EnemyStates[currentState](EntityID, dt)
end
```

---

## Quick Reference

### Component Getters (Current Entity)
- `GetTransform()`, `GetRigidBody()`, `GetSprite()`, `GetCollider()`, `GetPlayer()`, `GetEnemy()`, `GetCamera()`, `GetPathFinding()`, `GetProjectile()`

### Component Checkers (Current Entity)
- `HasTransform()`, `HasRigidBody()`, `HasSprite()`, `HasCollider()`, `HasPlayer()`, `HasEnemy()`, `HasCamera()`, `HasPathFinding()`, `HasProjectile()`

### Cross-Entity Access
- `GetEntity(id)` - Returns entity wrapper
- `Get[Component]From(id)` - Direct component access
- `Has[Component]On(id)` - Check if entity has component

### Entity Management
- `CreateEntity()`, `DestroyEntity(id)`, `DestroyWithChildren(id)`
- `SetParent(child, parent)`, `RemoveParent(child)`, `GetParent(id)`, `HasParent(id)`, `GetChildren(id)`
- `SetActiveEntity(entity, isActive)`
- `SpawnPrefab(prefabName, position)`

### Entity Queries
- `FindEntitiesWithComponent(name)`, `FindEntityWithComponent(name)`
- `GetEntityCount()`, `IsEntityValid(id)`

### Input
- `KeyDown(key)`, `KeyPressed(key)`, `KeyReleased(key)`
- `MouseButtonDown(btn)`, `MouseButtonPressed(btn)`, `MouseButtonReleased(btn)`
- `GetMousePosition()`

### Audio
- `PlaySound(name, volume, loops)`, `StopSound(name)`
- `PlayMusic(name, volume, loops)`, `StopMusic(name)`
- `PlayEntitySound(entity, name, loop, volume)`, `StopEntitySound(entity)`
- `StopEntitySoundByName(entity, soundName)`
- `PlayOneShotAtEntity(entity, name, volume)`
- `PlayOneShotAtPosition(x, y, name, volume)`

### Utility
- `Log(msg)`, `LogWarning(msg)`, `LogError(msg)`
- `GetDeltaTime()`
- `PlayAnimation(entity, animationName)`
- `AddForce(entity, position, direction, force, rotation)`
- `SetPathFindingGoal(entity, x, y)`
- `LoadScene(sceneName)`, `CloseApplication()`

### Lifecycle Callbacks
- `Start()`, `Update(dt)`, `OnEnable()`, `OnDisable()`, `OnDestroy()`, `OnClicked()`
- `OnCollisionEnter(other)`, `OnCollision(other)`, `OnCollisionExit(other)`
- `OnTriggerEnter(other)`, `OnTrigger(other)`, `OnTriggerExit(other)`

---

## Summary of NEW Features (Latest Update)

### New Components
- **PathFinding**: AI pathfinding with goal tracking
- **Projectile**: Projectile-specific properties (damage, speed, lifetime, fade)

### New Audio Functions
- **Spatial Audio**: Entity-attached sounds and positional audio
- `PlayEntitySound()`, `StopEntitySound()`, `StopEntitySoundByName()`
- `PlayOneShotAtEntity()`, `PlayOneShotAtPosition()`

### New Utility Functions
- **Animation**: `PlayAnimation(entity, animationName)`
- **Physics**: `AddForce(entity, position, direction, force, rotation)`
- **Pathfinding**: `SetPathFindingGoal(entity, x, y)`
- **Prefabs**: `SpawnPrefab(prefabName, position)`
- **Scene Management**: `LoadScene(sceneName)`, `CloseApplication()`
- **Entity Control**: `SetActiveEntity(entity, isActive)`

### New Lifecycle Callbacks
- **UI Events**: `OnClicked()` for button interactions

### Enhanced Component Access
- All new components available through both wrapper and direct access methods
- PathFinding and Projectile added to entity query system

---

**Document Version:** 2.0  
**Last Updated:** 2024  
**Engine:** Uma Engine  
**Scripting Language:** Lua 5.4

For additional help, refer to the engine source code or contact the development team.
