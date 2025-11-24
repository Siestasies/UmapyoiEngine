# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## IMPORTANT: Build and Testing Policy

**DO NOT run build scripts, CMake, or compile the project.** You will write/modify code and the user will verify builds themselves. Focus on code correctness, not build verification.

## Build System Reference

The project uses CMake with Visual Studio on Windows:
- Root `CMakeLists.txt` fetches dependencies (GLFW, GLM, stb, Lua 5.4.6, Sol2, GLAD, ImGui, FMOD, FreeType)
- `Engine/CMakeLists.txt` builds the Uma_Engine static library
- `Game/CMakeLists.txt` builds the UmapyoiGame executable and copies Assets/Logs/Configs folders + DLLs

C++20 standard is required. MSVC uses `/W4` with specific warning suppressions.

## Architecture Overview

### ECS (Entity Component System)

The core architecture follows a pure ECS pattern with centralized coordination:

**Coordinator** (`Engine/ECS/Core/Coordinator.hpp`)
- Central facade unifying EntityManager, ComponentManager, and SystemManager
- All ECS operations go through the Coordinator
- Handles entity/component lifecycle, system registration, and signature-based filtering
- Implements parent-child entity hierarchy with `SetParent()`, `GetChildren()`, `DestroyEntityAndChildren()`
- Has deletion queue (`ProcessDeletionQueue()`) for safe entity destruction
- Supports state caching for editor play mode (`CacheState()`, `RestoreState()`)
- Integrates with EventSystem to emit entity lifecycle events

**Components** (`Engine/ECS/Components/`)
- Transform, RigidBody, Sprite, Collider, Camera, Player, Enemy, Animator, LuaScript, AudioComponent, AudioListener, PathFinding
- When adding new components, register them in `Coordinator::Init()` and update `ForEachComponent()` macro list

**Systems** (`Engine/ECS/Systems/`)
- Physics, Collision, Rendering, Animation, Audio, Camera, PlayerController, PathFinding, LuaScripting
- Systems operate on entities matching their signature (bitset of required components)
- Register systems via `Coordinator::RegisterSystem<T>()` and set their signature

### Editor System

**Command Pattern for Undo/Redo** (`Engine/Editor/`)
- `ICommand` interface: `Execute()`, `Undo()`, `GetDescription()`
- `CommandHistory`: manages undo/redo stack
- Commands in `Engine/Editor/Cmds/`: EntityCreateCmd, EntityDeleteCmd, EntityDuplicateCmd, EntitySnapshotCmd
- When implementing new editor operations, create a command class and use `CommandHistory::Execute()`

**EditorSystem** (`Engine/Editor/Core/EditorSystem.h`)
- Coordinates entity picking, gizmo rendering, and transform manipulation
- Uses PickingSystem (raycasting), GizmoRenderer, and TransformManipulator subsystems
- Supports three manipulation modes: Translate, Rotate, Scale
- Disabled during play mode

### Core Systems Architecture

**SystemManager** (`Engine/Core/SystemManager.h`)
- Manages lifecycle of all engine systems (Init, Update, Shutdown)
- All systems inherit from `ISystem` interface
- Systems are registered in `Game/main.cpp` in specific order (EventSystem first, then InputSystem, etc.)

**EventSystem** (`Engine/Core/EventSystem.h`)
- Observer pattern for decoupled communication
- Event types defined in `Engine/Core/EventType.h` and event data in `Engine/Events/`
- Systems can inherit from `EventListenerSystem` to subscribe to events
- Key events: InputEvents (mouse, keyboard), EditorEvents (entity picked), ECSEvents (entity created/destroyed)

**SceneManager** (`Engine/Systems/SceneManager.h`)
- Manages scene loading/unloading via `SceneType` interface
- Scenes implement `Load()`, `Update()`, `Unload()`, `OnEnter()`, `OnExit()`
- Scenes are registered in main.cpp (EditorScript, GameSceneScript, etc.)

### Graphics Pipeline

**Graphics System** (`Engine/Systems/Graphics.hpp`)
- OpenGL-based rendering with GLFW window management
- Manages shaders, textures, framebuffers, VAO/VBO
- RenderingSystem uses Graphics to draw sprites/entities

**ResourcesManager** (`Engine/Systems/ResourcesManager.hpp`)
- Centralized asset loading: textures, shaders, audio, fonts
- Uses stb_image for texture loading, FreeType for fonts

### Scripting

**Lua Integration** (`Engine/ECS/Systems/LuaScriptingSystem.hpp`)
- Uses Sol2 (C++ binding for Lua 5.4.6)
- Entities can have LuaScript component with Init/Update/Shutdown callbacks
- Lua scripts can access ECS (get entities by component, add/remove components)
- Exposed to Lua: Transform, RigidBody, Sprite, Collider, Input, Audio, Scene management

### Project Structure

```
Engine/          - Core engine library (static lib)
  Core/          - SystemManager, EventSystem, serialization
  ECS/           - Entity Component System implementation
    Core/        - Coordinator, managers, types
    Components/  - Component definitions
    Systems/     - ECS systems (physics, rendering, etc.)
  Editor/        - In-engine editor with undo/redo
    Core/        - EditorSystem, command pattern
    Cmds/        - Command implementations
    Systems/     - Picking, gizmo, transform manipulation
  Systems/       - Engine-level systems (Graphics, Window, Input, Sound, Resources, Scene)
  Events/        - Event type definitions
  WIP_Scripts/   - Scene implementations, ImGui manager
  Math/          - Math utilities
  glad/          - OpenGL loader
  imgui/         - ImGui library
  fmod/          - FMOD audio library
  freetype/      - FreeType font library

Game/            - Game executable
  main.cpp       - Entry point, system registration

Assets/          - Game assets (textures, audio, fonts, lua scripts)
Configs/         - JSON configuration files
Logs/            - Runtime logs
```

## Key Patterns and Conventions

### Adding a New Component
1. Create component struct in `Engine/ECS/Components/`
2. Register in `Coordinator::Init()` via `RegisterComponent<T>()`
3. Add to `ForEachComponent()` macro list in `Coordinator.hpp`
4. Implement serialization if needed (Serialize/Deserialize methods)
5. Create corresponding system in `Engine/ECS/Systems/` if needed

### Adding a New System
1. Inherit from `Uma_ECS::System` (ECS system) or `ISystem` (engine system)
2. Implement Init(), Update(float dt), Shutdown()
3. Register in main.cpp via `systemManager.RegisterSystem<T>()`
4. If ECS system: set signature via `coordinator->SetSystemSignature<T>(signature)`
5. If event-driven: inherit from `EventListenerSystem` and implement `RegisterEventListeners()`

### Serialization
- Uses RapidJSON for JSON serialization
- Implement `ISerializer` interface for scene/game state serialization
- `Coordinator::Serialize()` handles entity/component serialization
- Prefab support via `SerializePrefab()` / `DeserializePrefab()`

### Memory Management
- Uses smart pointers (unique_ptr, shared_ptr) for ownership
- MemoryManager available in DEBUG builds for leak detection
- Entity deletion uses deferred queue to avoid mid-iteration deletion

## Working with This Codebase

- The engine uses event-driven architecture - prefer events over direct system coupling
- All ECS operations must go through the Coordinator - never access managers directly
- Editor operations must use the Command pattern for undo/redo support
- Test changes by describing them to the user - do not attempt to build or run
- Parent-child entity relationships are managed by Coordinator - use provided hierarchy functions
- Scene state is serialized to JSON - changes to components require serialization updates
