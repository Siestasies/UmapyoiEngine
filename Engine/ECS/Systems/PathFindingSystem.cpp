#include "PathFindingSystem.hpp"

#include "Components/Transform.h"
#include "Components/RigidBody.h"
#include "Components/PathFinding.h"

#include "Core/Coordinator.hpp"
#include "Systems/Graphics.hpp"

#include "Events/InputEvents.h"
#include "Events/IMGUIEvents.h"
#include <GLFW/glfw3.h>

void Uma_ECS::PathFindingSystem::Init(Coordinator* c, Uma_Engine::EventSystem* es, Uma_Engine::Graphics* graphics)
{
    pCoordinator = c;
    pEventSystem = es;
    pGraphics = graphics;

    gridPathfinder = new Uma_Navigation::GridPathfinder(cellSize);

    //to be removed later
    // Subscribe to mouse clicks for pathfinding
    pEventSystem->Subscribe<Uma_Engine::SceneViewMouseEvent, PathFindingSystem>(
        [this](const Uma_Engine::SceneViewMouseEvent& e) {
            viewportMousePos = Vec2(e.x, e.y);
            });

    pEventSystem->Subscribe<Uma_Engine::MouseButtonEvent, PathFindingSystem>(
        [this](const Uma_Engine::MouseButtonEvent& e) {
            if (e.button != GLFW_MOUSE_BUTTON_RIGHT || e.action != GLFW_PRESS) return;

            auto& pfArray = pCoordinator->GetComponentArray<PathFinding>();
            if (pfArray.Has(playerID)) {
                auto& pf = pfArray.GetData(playerID);
                pf.goal = pGraphics->ScreenToWorld(Vec2(viewportMousePos.x,viewportMousePos.y));
                pf.pathUpdateTimer = pf.pathUpdateInterval + 0.1f; // Force immediate update
            }
        });

    pEventSystem->Subscribe<Uma_Engine::CallPathFindToBake, PathFindingSystem>(
        [this](const Uma_Engine::CallPathFindToBake& e)
        {
            isDirty = true;
        }
    );

    //eventListeners.push_back(
    //    pEventSystem->Subscribe<Uma_Engine::MouseButtonEvent>(
    //        [this](const Uma_Engine::MouseButtonEvent& e) {
    //            if (e.button != GLFW_MOUSE_BUTTON_LEFT || e.action != GLFW_PRESS) return;

    //            auto& pfArray = pCoordinator->GetComponentArray<PathFinding>();
    //            //if (pfArray.Has(playerID)) {
    //            //    auto& pf = pfArray.GetData(playerID);
    //            //    pf.goal = pGraphics->ScreenToWorld(Vec2(e.x, e.y));
    //            //    pf.pathUpdateTimer = pf.pathUpdateInterval + 0.1f; // Force immediate update
    //            //}
    //            if (!pfArray.Has(playerID)) return;
    //            for (auto const& entity : aEntities) {
    //                auto& pf = pfArray.GetData(entity);
    //                pf.goal = pfArray.GetData(playerID).goal;
    //                pf.pathUpdateTimer = pf.pathUpdateInterval + 0.1f;
    //            }
    //        })
    //);
}

void Uma_ECS::PathFindingSystem::Update(float dt)
{
    auto& tfArray = pCoordinator->GetComponentArray<Transform>();
    auto& rbArray = pCoordinator->GetComponentArray<RigidBody>();
    auto& pfArray = pCoordinator->GetComponentArray<PathFinding>();
    auto& colArray = pCoordinator->GetComponentArray<Collider>();
    auto& playerArray = pCoordinator->GetComponentArray<Player>();
    auto& enemyArray = pCoordinator->GetComponentArray<Enemy>();

    Vec2 playerPosition(0, 0);
    bool hasPlayer = false;
    float maxAgentRadius = cellSize; // Initialize with minimum

    // Calculate maximum agent radius across ALL entities
    for (auto const& entity : aEntities)
    {
        if (colArray.Has(entity) && tfArray.Has(entity)) {
            const auto& collider = colArray.GetData(entity);
            const auto& tf = tfArray.GetData(entity);

            // FIX #1: Check playerArray.Has(entity) instead of hasPlayer
            if (playerArray.Has(entity) && collider.shapes.size() > 1 && collider.shapes[1].isActive) {
                // For player, use ONLY shapes[1] (the navigation collider)
                const auto& navShape = collider.shapes[1];
                float sx = navShape.size.x * tf.scale.x;
                float sy = navShape.size.y * tf.scale.y;
                float playerRadius = (std::max)(sx, sy) * 0.5f;

                // FIX #2: Use max() to compare, not overwrite
                maxAgentRadius = (std::max)(maxAgentRadius, playerRadius);
            }
            else 
            {
                // For other entities, use largest shape
                for (const auto& shape : collider.shapes) 
                {
                    if (!shape.isActive) continue;

                    float sx = shape.size.x * tf.scale.x;
                    float sy = shape.size.y * tf.scale.y;
                    float candidateRadius = (std::max)(sx, sy) * 0.5f;

                    maxAgentRadius = (std::max)(maxAgentRadius, candidateRadius);
                }
            }
        }
    }

    /*std::cout << "[PathFinding] Max agent radius: " << maxAgentRadius
        << " world units" << std::endl;*/

    // Find player and rebuild pathfinder around them
    if (playerArray.Size() > 0) 
    {
        playerID = playerArray.GetEntity(0);
        hasPlayer = tfArray.Has(playerID) && playerArray.Has(playerID);

        if (hasPlayer) 
        {
            //playerPosition = tfArray.GetData(playerID).worldPosition;
            playerPosition = tfArray.GetData(playerID).position;

            static Vec2 lastRebuildCenter(0, 0);
            static bool needsFirstRebuild = true;

            if (needsFirstRebuild || isDirty) 
            {
                RebuildPathfinder(playerPosition, maxAgentRadius);
                lastRebuildCenter = playerPosition;
                needsFirstRebuild = false;
                if (isDirty)
                    isDirty = false;
            }
            else 
            {
                Vec2 delta = playerPosition - lastRebuildCenter;
                float distance = std::sqrt(delta.x * delta.x + delta.y * delta.y);

                if (distance > rebuildRadius * 0.5f) 
                {
                    RebuildPathfinder(playerPosition, maxAgentRadius);
                    lastRebuildCenter = playerPosition;
                }
            }
        }
    }

    const float goalDeadZone = cellSize * 2.0f;

    // Process pathfinding for all entities
    for (auto const& entity : aEntities)
    {
        auto& tf = tfArray.GetData(entity);
        auto& pf = pfArray.GetData(entity);
        auto& rb = rbArray.GetData(entity);

        bool isPlayer = playerArray.Has(entity);
        bool isEnemy = enemyArray.Has(entity);

        // Calculate current position with collider offset
        Vec2 currentPos = tf.position;
        if (isPlayer && colArray.Has(entity)) {
            const auto& collider = colArray.GetData(entity);

            // Player uses shapes[1] for pathfinding position
            if (collider.shapes.size() > 1 && collider.shapes[1].isActive) {
                Vec2 worldOffset = Vec2{
                    collider.shapes[1].offset.x * tf.scale.x,
                    collider.shapes[1].offset.y * tf.scale.y
                };
                currentPos = tf.position + worldOffset;
            }
        }

        // FIX #3: Calculate agent radius per entity, using shapes[1] for player
        float agentRad = cellSize;
        if (colArray.Has(entity)) {
            const auto& collider = colArray.GetData(entity);

            if (isPlayer && collider.shapes.size() > 1 && collider.shapes[1].isActive) {
                // For player, use ONLY shapes[1]
                const auto& navShape = collider.shapes[1];
                float sx = navShape.size.x * tf.scale.x;
                float sy = navShape.size.y * tf.scale.y;
                agentRad = (std::max)(sx, sy) * 0.5f;
            }
            else {
                // For other entities, use largest shape
                for (const auto& shape : collider.shapes) {
                    if (!shape.isActive) continue;

                    float sx = shape.size.x * tf.scale.x;
                    float sy = shape.size.y * tf.scale.y;
                    float candidateRadius = (std::max)(sx, sy) * 0.5f;

                    agentRad = (std::max)(agentRad, candidateRadius);
                }
            }
        }

        Vec2 toGoal = pf.goal - currentPos;
        float distToGoal = std::sqrt(toGoal.x * toGoal.x + toGoal.y * toGoal.y);

        if (isPlayer && distToGoal < goalDeadZone && !pf.hasValidPath) {
            pf.reachedGoal = true;
            rb.velocity = Vec2(0, 0);
            pf.pathUpdateTimer = 0.0f;
            continue;
        }

        pf.pathUpdateTimer += dt;
        if (pf.pathUpdateTimer >= pf.pathUpdateInterval) {
            pf.path = gridPathfinder->FindPath(currentPos, pf.goal, agentRad);
            pf.path = gridPathfinder->SmoothPath(pf.path);
            pf.pathIndex = 0;
            pf.hasValidPath = !pf.path.empty();
            pf.pathUpdateTimer = 0.0f;

            if (isPlayer) {
                pf.reachedGoal = false;
            }
        }

        // Follow the path
        if (pf.hasValidPath && pf.pathIndex < pf.path.size()) {
            Vec2 target = pf.path[pf.pathIndex];
            Vec2 direction = target - currentPos;
            float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

            float waypointTolerance = cellSize * 0.5f;
            if (isPlayer) {
                waypointTolerance = cellSize * 0.2f;
            }
            else if (isEnemy) {
                waypointTolerance = cellSize * 0.6f;
            }

            if (distance < waypointTolerance) {
                pf.pathIndex++;
                if (pf.pathIndex >= pf.path.size()) {
                    pf.reachedGoal = true;
                    pf.hasValidPath = false;
                    rb.velocity = Vec2(0, 0);
                }
            }
            else if (distance > 0.001f) {
                float spd = 50.0f;
                if (isPlayer) {
                    spd = 50.0f;
                }
                else if (isEnemy) {
                    spd = enemyArray.GetData(entity).mSpeed;
                }

                rb.velocity = Vec2(direction.x / distance * spd, direction.y / distance * spd);
            }
        }
        else {
            rb.velocity = Vec2(0, 0);

            if (isEnemy && distToGoal < goalDeadZone * 0.5f) {
                pf.reachedGoal = true;
            }
        }
    }
}




void Uma_ECS::PathFindingSystem::RebuildPathfinder(const Vec2& center, float maxAgentRadius)
{
    std::unordered_set<Uma_Navigation::GridCell, Uma_Navigation::GridCellHash> blocked;

    auto colliderEntities = pCoordinator->GetEntitiesByComponent<Collider>();
    auto& tfArray = pCoordinator->GetComponentArray<Transform>();
    auto& colliderArray = pCoordinator->GetComponentArray<Collider>();
    auto& spriteArray = pCoordinator->GetComponentArray<Sprite>();

    for (auto entity : colliderEntities) {
        if (!tfArray.Has(entity)) continue;

        const auto& transform = tfArray.GetData(entity);
        const auto& collider = colliderArray.GetData(entity);

        float dx = transform.position.x - center.x;
        float dy = transform.position.y - center.y;
        float distSq = dx * dx + dy * dy;

        if (distSq > rebuildRadius * rebuildRadius) continue;

        Vec2 spriteSize{ 1.0f, 1.0f };
        if (spriteArray.Has(entity))
        {
            auto& s = spriteArray.GetData(entity);
            if (s.texture)
            {
                spriteSize = s.texture->GetNativeSize();
            }
        }

        for (const auto& shape : collider.shapes) {
            if (!shape.isActive) continue;
            if (shape.purpose != ColliderPurpose::Environment) continue;

            Vec2 effectiveSize = shape.autoFitToSprite ? spriteSize : shape.size;

            if (effectiveSize.x == 0 || effectiveSize.y == 0) {
                effectiveSize = Vec2(5.0f, 5.0f);
            }

            Vec2 scaledSize = Vec2{
                effectiveSize.x * transform.scale.x,
                effectiveSize.y * transform.scale.y
            };

            Vec2 worldOffset = Vec2{
                shape.offset.x * transform.scale.x,
                shape.offset.y * transform.scale.y
            };

            Vec2 shapeCenter = transform.position + worldOffset;
            Vec2 halfSize = scaledSize * 0.5f;

            int minX = static_cast<int>(std::floor((shapeCenter.x - halfSize.x) / cellSize));
            int maxX = static_cast<int>(std::floor((shapeCenter.x + halfSize.x) / cellSize));
            int minY = static_cast<int>(std::floor((shapeCenter.y - halfSize.y) / cellSize));
            int maxY = static_cast<int>(std::floor((shapeCenter.y + halfSize.y) / cellSize));

            for (int y = minY; y <= maxY; ++y) {
                for (int x = minX; x <= maxX; ++x) {
                    blocked.insert({ x, y });
                }
            }
        }
    }

    gridPathfinder->SetBlockedCells(blocked);

    // Calculate cells needed for largest agent
    int cellsNeeded = static_cast<int>(std::ceil(maxAgentRadius / cellSize));
    int maxClearanceRadius = cellsNeeded + 2;
    
    /*std::cout << "[PathFinding] Computing clearances: max agent radius="
        << maxAgentRadius << " world units, "
        << cellsNeeded << " cells, limit="
        << maxClearanceRadius << " cells" << std::endl;*/

    gridPathfinder->ComputeClearances(maxClearanceRadius);
}




void Uma_ECS::PathFindingSystem::Shutdown()
{
    delete gridPathfinder;
    gridPathfinder = nullptr;

    //to be removed later
    pEventSystem->UnsubscribeSystem<PathFindingSystem>();
}

void Uma_ECS::PathFindingSystem::DebugDraw()
{
    if (!pGraphics) return;

    auto& tfArray = pCoordinator->GetComponentArray<Transform>();
    auto& pfArray = pCoordinator->GetComponentArray<PathFinding>();
    auto& colliderArray = pCoordinator->GetComponentArray<Collider>();

    // 1. Draw all collider bounding boxes
    auto colliderEntities = pCoordinator->GetEntitiesByComponent<Collider>();
    for (auto entity : colliderEntities) {
        if (!tfArray.Has(entity)) continue;

        const auto& transform = tfArray.GetData(entity);
        const auto& collider = colliderArray.GetData(entity);

        for (const auto& shape : collider.shapes) {
            if (!shape.isActive) continue;

            // Draw collider bounds in red for walls, yellow for player
            float r = (entity == playerID) ? 1.0f : 1.0f;
            float g = (entity == playerID) ? 1.0f : 0.0f;
            float b = 0.0f;

            pGraphics->DrawDebugRect(transform.position, shape.size, r, g, b);
        }
    }

    // 2. Draw blocked grid cells (optional - can be expensive)
    if (gridPathfinder && true) { // Set to true to enable
        const auto& blockedCells = gridPathfinder->GetBlockedCells();
        for (const auto& cell : blockedCells) {
            Vec2 cellCenter(cell.x * cellSize + cellSize * 0.5f,
                cell.y * cellSize + cellSize * 0.5f);
            Vec2 cellSizeVec(cellSize, cellSize);
            pGraphics->DrawDebugRect(cellCenter, cellSizeVec, 0.5f, 0.0f, 0.0f);
        }
    }

    // 3. Draw paths and goals for all pathfinding entities
    for (auto const& entity : aEntities)
    {
        auto& tf = tfArray.GetData(entity);
        auto& pf = pfArray.GetData(entity);

        // Draw goal position in green
        Vec2 goalMarkerSize(8.0f, 8.0f);
        pGraphics->DrawDebugRect(pf.goal, goalMarkerSize, 0.0f, 1.0f, 0.0f);

        // Draw path waypoints
        if (pf.hasValidPath && !pf.path.empty()) {
            for (size_t i = pf.pathIndex; i < pf.path.size(); ++i) {
                Vec2 waypointSize(4.0f, 4.0f);

                // Current waypoint in cyan, others in blue
                if (i == pf.pathIndex) {
                    pGraphics->DrawDebugRect(pf.path[i], waypointSize, 0.0f, 1.0f, 1.0f);
                }
                else {
                    pGraphics->DrawDebugRect(pf.path[i], waypointSize, 0.3f, 0.3f, 1.0f);
                }
            }
        }

        // Draw current position in white
        Vec2 posMarkerSize(6.0f, 6.0f);
        pGraphics->DrawDebugRect(tf.position, posMarkerSize, 1.0f, 1.0f, 1.0f);
    }
}
