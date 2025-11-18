#include "PathFindingSystem.hpp"

#include "Components/Transform.h"
#include "Components/RigidBody.h"
#include "Components/PathFinding.h"

#include "Core/Coordinator.hpp"
#include "Systems/Graphics.hpp"
#include "../pathfinding/NavMeshTypes.h"

#include "Events/InputEvents.h"
#include <GLFW/glfw3.h>

void Uma_ECS::PathFindingSystem::Init(Coordinator* c, Uma_Engine::EventSystem* es, Uma_Engine::Graphics* graphics)
{
    pCoordinator = c;
    pEventSystem = es;
    pGraphics = graphics;

    gridPathfinder = new Uma_Navigation::GridPathfinder(cellSize);

    // Subscribe to mouse clicks for pathfinding
    pEventSystem->Subscribe<Uma_Engine::MouseButtonEvent>(
        [this](const Uma_Engine::MouseButtonEvent& e) {
            if (e.button != GLFW_MOUSE_BUTTON_RIGHT || e.action != GLFW_PRESS) return;

            auto& pfArray = pCoordinator->GetComponentArray<PathFinding>();
            if (pfArray.Has(playerID)) {
                auto& pf = pfArray.GetData(playerID);
                pf.goal = pGraphics->ScreenToWorld(Vec2(e.x, e.y));
                pf.pathUpdateTimer = pf.pathUpdateInterval + 0.1f; // Force immediate update
            }
        });
}

void Uma_ECS::PathFindingSystem::Update(float dt)
{
    static bool printedOnce = false;
    if (!printedOnce) {
        Uma_Navigation::GridCell testCell{ 4, 0 };
        Vec2 worldPos = gridPathfinder->GridToWorld(testCell);
        std::cout << "Cell (4,0) converts to world pos: (" << worldPos.x << ", " << worldPos.y << ")" << std::endl;
        std::cout << "Expected: (20, 0) for corner-aligned" << std::endl;
        printedOnce = true;
    }

    auto& tfArray = pCoordinator->GetComponentArray<Transform>();
    auto& rbArray = pCoordinator->GetComponentArray<RigidBody>();
    auto& pfArray = pCoordinator->GetComponentArray<PathFinding>();
    auto& playerArray = pCoordinator->GetComponentArray<Player>();
    auto& enemyArray = pCoordinator->GetComponentArray<Enemy>();
    auto& colliderArray = pCoordinator->GetComponentArray<Collider>();

    Vec2 playerPosition(0, 0);
    bool hasPlayer = false;

    // Find player and rebuild pathfinder around them
    if (playerArray.Size() > 0) {
        playerID = playerArray.GetEntity(0);
        hasPlayer = tfArray.Has(playerID) && playerArray.Has(playerID);

        if (hasPlayer) {
            playerPosition = tfArray.GetData(playerID).position;

            static Vec2 lastRebuildCenter(0, 0);
            static bool needsFirstRebuild = true;

            if (needsFirstRebuild) {
                RebuildPathfinder(playerPosition);
                lastRebuildCenter = playerPosition;
                needsFirstRebuild = false;
            }
            else {
                Vec2 delta = playerPosition - lastRebuildCenter;
                float distance = std::sqrt(delta.x * delta.x + delta.y * delta.y);

                if (distance > 50.0f) {
                    RebuildPathfinder(playerPosition);
                    lastRebuildCenter = playerPosition;
                }
            }
        }
    }

    // FIX: Increase goal dead zone to 2x cellSize
    const float goalDeadZone = cellSize * 2.0f; // 10 pixels instead of 7.5

    // Process pathfinding for all entities
    for (auto const& entity : aEntities)
    {
        auto& tf = tfArray.GetData(entity);
        auto& pf = pfArray.GetData(entity);
        auto& rb = rbArray.GetData(entity);

        // Get foot offset
        float footOffsetY = 5.0f;
        if (colliderArray.Has(entity)) {
            const auto& collider = colliderArray.GetData(entity);
            if (!collider.shapes.empty()) {
                footOffsetY = collider.shapes[1].offset.y;
            }
        }

        // Use FOOT position for pathfinding, not sprite center
        Vec2 footPosition = Vec2(tf.position.x, tf.position.y + footOffsetY);

        // Calculate distance to goal from FEET
        Vec2 toGoal = pf.goal - footPosition;
        float distToGoal = std::sqrt(toGoal.x * toGoal.x + toGoal.y * toGoal.y);

        const float goalDeadZone = cellSize * 2.0f;

        if (distToGoal < goalDeadZone && !pf.hasValidPath) {
            pf.reachedGoal = true;
            rb.velocity = Vec2(0, 0);
            pf.pathUpdateTimer = 0.0f;
            continue;
        }

        if (!pf.hasValidPath || pf.pathIndex >= pf.path.size()) {
            pf.pathUpdateTimer += dt;
        }

        if (pf.pathUpdateTimer >= pf.pathUpdateInterval) {
            // Find path FROM foot position, not sprite center
            pf.path = gridPathfinder->FindPath(footPosition, pf.goal);

            if (pf.path.size() > 4) {
                pf.path = SmoothPath(pf.path);
            }

            // Adjust waypoints to account for foot offset
            // When sprite center reaches waypoint, feet should be at waypoint
            for (auto& waypoint : pf.path) {
                waypoint.y -= footOffsetY; // Shift up so feet land on target
            }

            pf.pathIndex = 0;
            pf.hasValidPath = !pf.path.empty();
            pf.pathUpdateTimer = 0.0f;
            pf.reachedGoal = false;
        }

        // Follow the path (using sprite center, but waypoints are adjusted)
        if (pf.hasValidPath && pf.pathIndex < pf.path.size()) {
            Vec2 target = pf.path[pf.pathIndex];
            Vec2 direction = target - tf.position; // Sprite center to waypoint
            float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

            if (distance < cellSize) {
                pf.pathIndex++;
                if (pf.pathIndex >= pf.path.size()) {
                    pf.reachedGoal = true;
                    pf.hasValidPath = false;
                    rb.velocity = Vec2(0, 0);
                }
            }
            else if (distance > 0.001f) {
                float spd = 50.0f;
                if (playerArray.Has(entity)) {
                    spd = 50.0f;
                }
                else if (enemyArray.Has(entity)) {
                    spd = enemyArray.GetData(entity).mSpeed;
                }

                rb.velocity = Vec2(direction.x / distance * spd, direction.y / distance * spd);
            }
        }
        else {
            rb.velocity = Vec2(0, 0);
        }
    }
}




void Uma_ECS::PathFindingSystem::RebuildPathfinder(const Vec2& center)
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

            // NO INFLATION - just block where walls actually are
            Uma_Navigation::GridCell minCell = gridPathfinder->WorldToGrid(
                Vec2(shapeCenter.x - halfSize.x, shapeCenter.y - halfSize.y)
            );

            Uma_Navigation::GridCell maxCell = gridPathfinder->WorldToGrid(
                Vec2(shapeCenter.x + halfSize.x, shapeCenter.y + halfSize.y)
            );

            for (int y = minCell.y; y <= maxCell.y; ++y) {
                for (int x = minCell.x; x <= maxCell.x; ++x) {
                    blocked.insert({ x, y });
                }
            }
        }
    }

    gridPathfinder->SetBlockedCells(blocked);
}



void Uma_ECS::PathFindingSystem::Shutdown()
{
    delete gridPathfinder;
    gridPathfinder = nullptr;
}

void Uma_ECS::PathFindingSystem::DebugDraw()
{
    if (!pGraphics) return;

    auto& tfArray = pCoordinator->GetComponentArray<Transform>();
    auto& pfArray = pCoordinator->GetComponentArray<PathFinding>();
    auto& colliderArray = pCoordinator->GetComponentArray<Collider>();
    auto& spriteArray = pCoordinator->GetComponentArray<Sprite>();

    // 1. Draw ACTUAL collision boxes (where collision happens)
    auto colliderEntities = pCoordinator->GetEntitiesByComponent<Collider>();
    for (auto entity : colliderEntities) {
        if (!tfArray.Has(entity)) continue;

        const auto& transform = tfArray.GetData(entity);
        const auto& collider = colliderArray.GetData(entity);

        // Get sprite size for autoFitToSprite
        Vec2 spriteSize{ 1.0f, 1.0f };
        if (spriteArray.Has(entity))
        {
            auto& s = spriteArray.GetData(entity);
            if (s.texture)
            {
                spriteSize = s.texture->GetNativeSize();

                if (spriteSize.x > cellSize) spriteSize.x = cellSize;
                if (spriteSize.y > cellSize) spriteSize.y = cellSize;
            }
        }

        for (const auto& shape : collider.shapes) {
            if (!shape.isActive) continue;

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

            Vec2 colliderCenter = transform.position + worldOffset;

            // Draw ACTUAL collision box in yellow
            float r = (entity == playerID) ? 1.0f : 1.0f;
            float g = (entity == playerID) ? 1.0f : 1.0f;
            float b = 0.0f;

            pGraphics->DrawDebugRect(colliderCenter, scaledSize, r, g, b);
        }
    }

    // 2. Draw blocked grid cells aligned to COLLISION boxes
    if (gridPathfinder && true) {
        const auto& blockedCells = gridPathfinder->GetBlockedCells();
        for (const auto& cell : blockedCells) {
            Vec2 cellCorner(cell.x * cellSize, cell.y * cellSize);
            Vec2 cellCenter(cellCorner.x + cellSize * 0.5f, cellCorner.y + cellSize * 0.5f);
            Vec2 cellSizeVec(cellSize, cellSize);

            // Draw in semi-transparent red
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


std::vector<Vec2> Uma_ECS::PathFindingSystem::SmoothPath(const std::vector<Vec2>& path)
{
    if (path.size() <= 3) return path; // Don't smooth very short paths

    std::vector<Vec2> smoothed;
    smoothed.push_back(path.front()); // Keep start

    // Keep every 2nd waypoint
    for (size_t i = 2; i < path.size() - 1; i += 2) {
        smoothed.push_back(path[i]);
    }

    if (path.size() > 1) {
        smoothed.push_back(path.back());
    }

    return smoothed;
}

