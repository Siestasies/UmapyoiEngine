# Lua Scripting System API Documentation

## Table of Contents
1. [Lifecycle Functions](#lifecycle-functions)
2. [Component Access](#component-access)
3. [Entity Management](#entity-management)
4. [Input System](#input-system)
5. [Utility Functions](#utility-functions)
6. [Event Callbacks](#event-callbacks)
7. [Data Types](#data-types)

---

## Lifecycle Functions

These functions are automatically called by the engine at specific points in the entity's lifecycle. Define them in your Lua script to hook into these events.

### `Start()`
Called once when the script is first initialized and enabled.

```lua
function Start()
    -- Initialize your script here
end
```

### `Update(dt)`
Called every frame while the script is enabled.

**Parameters:**
- `dt` (float): Delta time since last frame

```lua
function Update(dt)
    -- Per-frame logic here
end
```

### `OnEnable()`
Called when the script is enabled (including initial enable).

```lua
function OnEnable()
    -- Called when script becomes active
end
```

### `OnDisable()`
Called when the script is disabled.

```lua
function OnDisable()
    -- Called when script becomes inactive
end
```

### `OnDestroy()`
Called when the entity or script is destroyed.

```lua
function OnDestroy()
    -- Cleanup logic here
end
```

---

## Component Access

### Current Entity Components

Access components attached to the current entity:

#### `GetTransform()`
Returns the Transform component of the current entity.

**Returns:** `Transform` reference or `nil`

```lua
local transform = GetTransform()
if transform then
    transform.position.x = 10
    transform.position.y = 20
    transform.rotation = 45
    transform.scale.x = 2
    transform.scale.y = 2
end
```

#### `HasTransform()`
Checks if the current entity has a Transform component.

**Returns:** `boolean`

```lua
if HasTransform() then
    -- Safe to use GetTransform()
end
```

#### `GetRigidBody()`
Returns the RigidBody component of the current entity.

**Returns:** `RigidBody` reference or `nil`

```lua
local rb = GetRigidBody()
if rb then
    rb.velocity.x = 5
    rb.velocity.y = 0
    rb.acceleration.x = 0.5
    rb.accel_strength = 100
    rb.fric_coeff = 0.9
end
```

#### `HasRigidBody()`
Checks if the current entity has a RigidBody component.

**Returns:** `boolean`

#### `GetSprite()`
Returns the Sprite component of the current entity.

**Returns:** `Sprite` reference or `nil`

```lua
local sprite = GetSprite()
if sprite then
    sprite.textureName = "player_idle.png"
    sprite.renderLayer = 1
    sprite.flipX = false
    sprite.flipY = false
end
```

#### `HasSprite()`
Checks if the current entity has a Sprite component.

**Returns:** `boolean`

---

### Cross-Entity Component Access

Two methods to access components on other entities:

#### Method 1: Entity Wrapper (Object-Oriented Style)

##### `GetEntity(entityId)`
Returns an entity wrapper object with component access methods.

**Parameters:**
- `entityId` (Entity): The ID of the target entity

**Returns:** `table` with the following structure:
- `id`: The entity ID
- `isValid`: Whether the entity exists and is active
- `GetTransform()`: Get Transform component
- `HasTransform()`: Check for Transform component
- `GetRigidBody()`: Get RigidBody component
- `HasRigidBody()`: Check for RigidBody component
- `GetSprite()`: Get Sprite component
- `HasSprite()`: Check for Sprite component
- `GetCollider()`: Get Collider component
- `HasCollider()`: Check for Collider component
- `GetPlayer()`: Get Player component
- `HasPlayer()`: Check for Player component
- `GetEnemy()`: Get Enemy component
- `HasEnemy()`: Check for Enemy component
- `GetCamera()`: Get Camera component
- `HasCamera()`: Check for Camera component

```lua
local otherEntity = GetEntity(targetEntityId)
if otherEntity.isValid then
    if otherEntity.HasTransform() then
        local transform = otherEntity.GetTransform()
        transform.position.x = 100
    end
end
```

#### Method 2: Direct Functions (Functional Style)

##### `GetTransformFrom(entityId)`
Gets the Transform component from a specific entity.

**Parameters:**
- `entityId` (Entity): The target entity ID

**Returns:** `Transform` reference or `nil`

```lua
local transform = GetTransformFrom(otherEntityId)
if transform then
    Log("Other entity position: " .. transform.position.x .. ", " .. transform.position.y)
end
```

##### `HasTransformOn(entityId)`
Checks if a specific entity has a Transform component.

**Parameters:**
- `entityId` (Entity): The target entity ID

**Returns:** `boolean`

Similar functions exist for all component types:
- `GetRigidBodyFrom(entityId)` / `HasRigidBodyOn(entityId)`
- `GetSpriteFrom(entityId)` / `HasSpriteOn(entityId)`
- `GetColliderFrom(entityId)` / `HasColliderOn(entityId)`
- `GetPlayerFrom(entityId)` / `HasPlayerOn(entityId)`
- `GetEnemyFrom(entityId)` / `HasEnemyOn(entityId)`
- `GetCameraFrom(entityId)` / `HasCameraOn(entityId)`

---

## Entity Management

### Entity Queries

#### `FindEntitiesWithComponent(componentName)`
Finds all entities that have a specific component.

**Parameters:**
- `componentName` (string): Name of the component (e.g., "Transform", "Player")

**Returns:** `table` (array of entity IDs)

```lua
local players = FindEntitiesWithComponent("Player")
for i, entityId in ipairs(players) do
    local transform = GetTransformFrom(entityId)
    if transform then
        Log("Player at: " .. transform.position.x .. ", " .. transform.position.y)
    end
end
```

#### `FindEntityWithComponent(componentName)`
Finds the first entity that has a specific component.

**Parameters:**
- `componentName` (string): Name of the component

**Returns:** `Entity` ID or `-1` if not found

```lua
local playerEntity = FindEntityWithComponent("Player")
if playerEntity ~= -1 then
    local transform = GetTransformFrom(playerEntity)
    -- Do something with player
end
```

#### `GetEntityCount()`
Gets the total number of active entities.

**Returns:** `integer`

```lua
local count = GetEntityCount()
Log("Total entities: " .. count)
```

#### `IsEntityValid(entityId)`
Checks if an entity ID is valid and active.

**Parameters:**
- `entityId` (Entity): The entity ID to check

**Returns:** `boolean`

```lua
if IsEntityValid(targetEntity) then
    -- Safe to access this entity
end
```

### Current Entity

#### `EntityID`
A global variable available in each script containing the current entity's ID.

```lua
Log("My entity ID is: " .. EntityID)
```

---

## Input System

### Keyboard Input

#### `KeyDown(keyCode)`
Checks if a key is currently held down.

**Parameters:**
- `keyCode` (integer): Key constant (see Key Constants)

**Returns:** `boolean`

```lua
if KeyDown(KEY_W) then
    -- Move forward
end
```

#### `KeyPressed(keyCode)`
Checks if a key was just pressed this frame.

**Parameters:**
- `keyCode` (integer): Key constant

**Returns:** `boolean`

```lua
if KeyPressed(KEY_SPACE) then
    -- Jump
end
```

#### `KeyReleased(keyCode)`
Checks if a key was just released this frame.

**Parameters:**
- `keyCode` (integer): Key constant

**Returns:** `boolean`

```lua
if KeyReleased(KEY_SHIFT) then
    -- Stop sprinting
end
```

### Mouse Input

#### `MouseButtonDown(buttonCode)`
Checks if a mouse button is currently held down.

**Parameters:**
- `buttonCode` (integer): Mouse button constant

**Returns:** `boolean`

```lua
if MouseButtonDown(MOUSE_LEFT) then
    -- Firing weapon
end
```

#### `MouseButtonPressed(buttonCode)`
Checks if a mouse button was just pressed this frame.

**Parameters:**
- `buttonCode` (integer): Mouse button constant

**Returns:** `boolean`

```lua
if MouseButtonPressed(MOUSE_RIGHT) then
    -- Aim
end
```

#### `MouseButtonReleased(buttonCode)`
Checks if a mouse button was just released this frame.

**Parameters:**
- `buttonCode` (integer): Mouse button constant

**Returns:** `boolean`

#### `GetMousePosition()`
Gets the current mouse position in world coordinates.

**Returns:** `Vec2` (x, y coordinates)

```lua
local mousePos = GetMousePosition()
Log("Mouse at: " .. mousePos.x .. ", " .. mousePos.y)
```

### Key Constants

Pre-defined constants for keyboard and mouse input:

**Keyboard:**
- `KEY_W`, `KEY_A`, `KEY_S`, `KEY_D`
- `KEY_SPACE`
- `KEY_SHIFT`
- `KEY_CTRL`
- `KEY_E`

**Mouse:**
- `MOUSE_LEFT`
- `MOUSE_RIGHT`
- `MOUSE_MIDDLE`

```lua
function Update(dt)
    if KeyDown(KEY_W) then
        local rb = GetRigidBody()
        rb.velocity.y = -5
    end
end
```

---

## Utility Functions

### Logging

#### `Log(message)`
Logs an informational message.

**Parameters:**
- `message` (string): The message to log

```lua
Log("Player health: " .. health)
```

#### `LogWarning(message)`
Logs a warning message.

**Parameters:**
- `message` (string): The warning message

```lua
LogWarning("Player health is low!")
```

#### `LogError(message)`
Logs an error message.

**Parameters:**
- `message` (string): The error message

```lua
LogError("Failed to load resource!")
```

### Time

#### `GetDeltaTime()`
Gets the time elapsed since the last frame.

**Returns:** `float` (seconds)

```lua
function Update(dt)
    -- dt and GetDeltaTime() return the same value
    local deltaTime = GetDeltaTime()
    Log("Frame time: " .. deltaTime)
end
```

---

## Event Callbacks

These are optional callback functions you can define to respond to physics events.

### Collision Events

#### `OnCollisionEnter(otherEntityId)`
Called when this entity starts colliding with another entity.

**Parameters:**
- `otherEntityId` (Entity): The ID of the entity we collided with

```lua
function OnCollisionEnter(other)
    Log("Started colliding with entity: " .. other)
    
    if HasPlayerOn(other) then
        Log("Hit the player!")
    end
end
```

#### `OnCollision(otherEntityId)`
Called every frame while this entity is colliding with another entity.

**Parameters:**
- `otherEntityId` (Entity): The ID of the entity we're colliding with

```lua
function OnCollision(other)
    -- Continuous collision logic
end
```

#### `OnCollisionExit(otherEntityId)`
Called when this entity stops colliding with another entity.

**Parameters:**
- `otherEntityId` (Entity): The ID of the entity we stopped colliding with

```lua
function OnCollisionExit(other)
    Log("Stopped colliding with entity: " .. other)
end
```

### Trigger Events

#### `OnTriggerEnter(otherEntityId)`
Called when this entity enters a trigger volume.

**Parameters:**
- `otherEntityId` (Entity): The ID of the trigger or entity

```lua
function OnTriggerEnter(other)
    Log("Entered trigger!")
end
```

#### `OnTrigger(otherEntityId)`
Called every frame while this entity is inside a trigger volume.

**Parameters:**
- `otherEntityId` (Entity): The ID of the trigger or entity

```lua
function OnTrigger(other)
    -- Inside trigger logic
end
```

#### `OnTriggerExit(otherEntityId)`
Called when this entity exits a trigger volume.

**Parameters:**
- `otherEntityId` (Entity): The ID of the trigger or entity

```lua
function OnTriggerExit(other)
    Log("Exited trigger!")
end
```

---

## Data Types

### Vec2

2D vector type for positions, velocities, etc.

**Constructor:**
```lua
local vec = Vec2()         -- (0, 0)
local vec = Vec2(10, 20)   -- (10, 20)
```

**Properties:**
- `x` (float): X component
- `y` (float): Y component

**Operators:**
```lua
local a = Vec2(1, 2)
local b = Vec2(3, 4)

local sum = a + b          -- Addition
local diff = a - b         -- Subtraction
local scaled = a * 2       -- Scalar multiplication
local scaled2 = 2 * a      -- Scalar multiplication (reversed)
```

**Usage:**
```lua
local transform = GetTransform()
transform.position = Vec2(100, 50)
transform.position.x = 200

local velocity = Vec2(5, -3)
local rb = GetRigidBody()
rb.velocity = velocity
```

### Transform

Component for entity position, rotation, and scale.

**Properties:**
- `position` (Vec2): World position
- `rotation` (float): Rotation in degrees
- `scale` (Vec2): Scale factors

```lua
local transform = GetTransform()
transform.position = Vec2(100, 100)
transform.rotation = 45
transform.scale = Vec2(2, 2)
```

### RigidBody

Component for physics simulation.

**Properties:**
- `velocity` (Vec2): Current velocity
- `acceleration` (Vec2): Current acceleration
- `accel_strength` (float): Acceleration strength
- `fric_coeff` (float): Friction coefficient

```lua
local rb = GetRigidBody()
rb.velocity = Vec2(10, 0)
rb.acceleration = Vec2(0, 0)
rb.accel_strength = 50
rb.fric_coeff = 0.8
```

### Sprite

Component for rendering sprites.

**Properties:**
- `textureName` (string): Name of the texture file
- `renderLayer` (integer): Render layer (higher = drawn on top)
- `flipX` (boolean): Flip horizontally
- `flipY` (boolean): Flip vertically

```lua
local sprite = GetSprite()
sprite.textureName = "character.png"
sprite.renderLayer = 2
sprite.flipX = false
sprite.flipY = false
```

---

## Exposed Variables

You can expose variables to the editor by declaring them in an `ExposedVars` table:

```lua
ExposedVars = {
    speed = 10.0,
    maxHealth = 100,
    isInvincible = false,
    playerName = "Hero"
}

function Start()
    Log("Speed: " .. speed)
    Log("Max Health: " .. maxHealth)
end

function Update(dt)
    -- Use the exposed variables
    local transform = GetTransform()
    if KeyDown(KEY_W) then
        transform.position.y = transform.position.y - speed * dt
    end
end
```

Supported types:
- `float` (number with decimal)
- `int` (whole number)
- `bool` (true/false)
- `string` (text)

---

## Complete Example Scripts

### Player Movement Controller

```lua
ExposedVars = {
    moveSpeed = 200.0,
    jumpForce = 500.0
}

function Start()
    Log("Player controller initialized")
end

function Update(dt)
    local rb = GetRigidBody()
    if not rb then return end
    
    -- Horizontal movement
    local moveX = 0
    if KeyDown(KEY_A) then
        moveX = -1
    elseif KeyDown(KEY_D) then
        moveX = 1
    end
    
    rb.velocity.x = moveX * moveSpeed
    
    -- Jump
    if KeyPressed(KEY_SPACE) then
        rb.velocity.y = -jumpForce
    end
end

function OnCollisionEnter(other)
    if HasEnemyOn(other) then
        LogWarning("Hit an enemy!")
    end
end
```

### Enemy AI

```lua
ExposedVars = {
    patrolSpeed = 50.0,
    detectionRange = 300.0
}

local direction = 1

function Start()
    Log("Enemy AI started")
end

function Update(dt)
    -- Find player
    local playerEntity = FindEntityWithComponent("Player")
    if playerEntity == -1 then return end
    
    local myTransform = GetTransform()
    local playerTransform = GetTransformFrom(playerEntity)
    
    if myTransform and playerTransform then
        local dx = playerTransform.position.x - myTransform.position.x
        local dy = playerTransform.position.y - myTransform.position.y
        local distance = math.sqrt(dx * dx + dy * dy)
        
        -- Chase player if in range
        if distance < detectionRange then
            local rb = GetRigidBody()
            if rb then
                rb.velocity.x = (dx > 0 and 1 or -1) * patrolSpeed
            end
        else
            -- Patrol
            Patrol(dt)
        end
    end
end

function Patrol(dt)
    local rb = GetRigidBody()
    if rb then
        rb.velocity.x = direction * patrolSpeed
    end
end

function OnCollisionEnter(other)
    -- Turn around on collision
    direction = -direction
end
```

### Collectible Item

```lua
ExposedVars = {
    points = 10
}

function OnTriggerEnter(other)
    if HasPlayerOn(other) then
        Log("Player collected item worth " .. points .. " points!")
        -- Item would be destroyed by C++ system
    end
end
```

---

## Best Practices

1. **Always check for nil**: Component getters return `nil` if the component doesn't exist
   ```lua
   local transform = GetTransform()
   if transform then
       -- Safe to use
   end
   ```

2. **Use Has functions**: Check if a component exists before getting it
   ```lua
   if HasRigidBody() then
       local rb = GetRigidBody()
       -- Use rb
   end
   ```

3. **Validate entity IDs**: Always check if entities are valid
   ```lua
   if IsEntityValid(targetEntity) then
       -- Safe to access
   end
   ```

4. **Cache component references**: Store frequently accessed components
   ```lua
   local myTransform = nil
   
   function Start()
       myTransform = GetTransform()
   end
   
   function Update(dt)
       if myTransform then
           -- Use cached reference
       end
   end
   ```

5. **Use ExposedVars for tuning**: Expose values you want to adjust in the editor
   ```lua
   ExposedVars = {
       speed = 100.0,
       damage = 25
   }
   ```

---

## Notes

- All component modifications are immediately reflected in the C++ engine
- The `Update()` function receives delta time in seconds
- Entity IDs are stable during the entity's lifetime
- Invalid entity operations are logged as errors but won't crash
- Scripts are hot-reloadable (exposed variables persist across reloads)
