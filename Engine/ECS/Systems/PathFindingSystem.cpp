#include "PathFindingSystem.hpp"

#include "Components/Transform.h"
#include "Components/RigidBody.h"
#include "Components/PathFinding.h"

#include "Core/Coordinator.hpp"
#include "Systems/Graphics.hpp"

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
    auto& tfArray = pCoordinator->GetComponentArray<Transform>();
    auto& rbArray = pCoordinator->GetComponentArray<RigidBody>();
    auto& pfArray = pCoordinator->GetComponentArray<PathFinding>();
    auto& playerArray = pCoordinator->GetComponentArray<Player>();
    auto& enemyArray = pCoordinator->GetComponentArray<Enemy>();

    Vec2 playerPosition(0, 0);
    float playerSpeed = 100.0f;
    bool hasPlayer = false;

    // Find player and rebuild pathfinder around them
    if (playerArray.Size() > 0) {
        playerID = playerArray.GetEntity(0);
        hasPlayer = tfArray.Has(playerID) && playerArray.Has(playerID);

        if (hasPlayer) {
            playerPosition = tfArray.GetData(playerID).position;
            playerSpeed = playerArray.GetData(playerID).mSpeed;

            // Rebuild pathfinder periodically
            static Vec2 lastRebuildCenter(0, 0);
            static bool needsFirstRebuild = true;

            if (needsFirstRebuild) {
                std::cout << "[PathFinding] Initial build at (" << playerPosition.x << ", " << playerPosition.y << ")" << std::endl;
                RebuildPathfinder(playerPosition);
                lastRebuildCenter = playerPosition;
                needsFirstRebuild = false;
            }
            else {
                Vec2 delta = playerPosition - lastRebuildCenter;
                float distance = std::sqrt(delta.x * delta.x + delta.y * delta.y);

                if (distance > 50.0f) {
                    std::cout << "[PathFinding] Rebuilding (moved " << distance << " units)" << std::endl;
                    RebuildPathfinder(playerPosition);
                    lastRebuildCenter = playerPosition;
                }
            }
        }
    }

    // Process pathfinding for all entities
    for (auto const& entity : aEntities)
    {
        auto& tf = tfArray.GetData(entity);
        auto& pf = pfArray.GetData(entity);
        auto& rb = rbArray.GetData(entity);

        // Update path periodically
        pf.pathUpdateTimer += dt;
        if (pf.pathUpdateTimer >= pf.pathUpdateInterval) {

            pf.path = gridPathfinder->FindPath(tf.position, pf.goal);
            pf.pathIndex = 0;
            pf.hasValidPath = !pf.path.empty();

            // Debug output for player
            if (entity == playerID) {
                std::cout << "[Player] Path from (" << tf.position.x << ", " << tf.position.y << ")"
                    << " to (" << pf.goal.x << ", " << pf.goal.y << ")" << std::endl;
                std::cout << "[Player] Waypoints: " << pf.path.size() << std::endl;

                if (!pf.path.empty()) {
                    for (size_t i = 0; i < (std::min)(pf.path.size(), (size_t)5); ++i) {
                        std::cout << "  [" << i << "] (" << pf.path[i].x << ", " << pf.path[i].y << ")" << std::endl;
                    }
                    if (pf.path.size() > 5) {
                        std::cout << "  ... and " << (pf.path.size() - 5) << " more" << std::endl;
                    }
                }
            }

            pf.pathUpdateTimer = 0.0f;
            pf.reachedGoal = false;
        }

        // Follow the path
        if (pf.hasValidPath && pf.pathIndex < pf.path.size()) {
            Vec2 target = pf.path[pf.pathIndex];
            Vec2 direction = target - tf.position;
            float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

            if (distance < 2.0f) {
                // Reached waypoint, move to next
                pf.pathIndex++;
                if (pf.pathIndex >= pf.path.size()) {
                    // Reached final goal
                    pf.reachedGoal = true;
                    pf.hasValidPath = false;
                    rb.velocity = Vec2(0, 0);
                }
            }
            else if (distance > 0.001f) {
                // Move toward waypoint
                float spd = 50.0f;
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

    std::cout << "[PathFinding] ========================================" << std::endl;
    std::cout << "[PathFinding] Rebuilding at center (" << center.x << ", " << center.y << ")" << std::endl;
    std::cout << "[PathFinding] Rebuild radius: " << rebuildRadius << std::endl;
    std::cout << "[PathFinding] Cell size: " << cellSize << std::endl;
    std::cout << "[PathFinding] Total collider entities: " << colliderEntities.size() << std::endl;

    int processedCount = 0;
    int skippedCount = 0;

    for (auto entity : colliderEntities) {
        if (entity == playerID) {
            std::cout << "[PathFinding] Skipping player entity: " << entity << std::endl;
            continue;
        }

        if (!tfArray.Has(entity)) {
            std::cout << "[PathFinding] Entity " << entity << " has no Transform!" << std::endl;
            continue;
        }

        const auto& transform = tfArray.GetData(entity);
        const auto& collider = colliderArray.GetData(entity);

        // Calculate distance
        float dx = transform.position.x - center.x;
        float dy = transform.position.y - center.y;
        float distSq = dx * dx + dy * dy;
        float dist = std::sqrt(distSq);

        std::cout << "[PathFinding] Entity " << entity
            << " at (" << transform.position.x << ", " << transform.position.y << ")"
            << " distance: " << dist << " units" << std::endl;

        if (distSq > rebuildRadius * rebuildRadius) {
            std::cout << "  -> SKIPPED (distance " << dist << " > radius " << rebuildRadius << ")" << std::endl;
            skippedCount++;
            continue;
        }

        // Process shapes
        for (const auto& shape : collider.shapes) {
            if (!shape.isActive) {
                std::cout << "  -> Shape INACTIVE" << std::endl;
                continue;
            }

            // === FALLBACK LOGIC GOES HERE ===
            Vec2 actualSize = shape.size;

            if (actualSize.x == 0 || actualSize.y == 0) {
                std::cout << "  -> Shape has zero size, using default 32x32" << std::endl;

                // Use default tile size (adjust this to match your actual tile size)
                actualSize = Vec2(5.0f, 5.0f);
            }

            Vec2 halfSize(actualSize.x * 0.5f, actualSize.y * 0.5f);
            // === END FALLBACK LOGIC ===

            int minX = static_cast<int>(std::floor((transform.position.x - halfSize.x) / cellSize));
            int maxX = static_cast<int>(std::ceil((transform.position.x + halfSize.x) / cellSize));
            int minY = static_cast<int>(std::floor((transform.position.y - halfSize.y) / cellSize));
            int maxY = static_cast<int>(std::ceil((transform.position.y + halfSize.y) / cellSize));

            int cellCount = (maxX - minX + 1) * (maxY - minY + 1);

            std::cout << "  -> BLOCKING cells (" << minX << "," << minY << ") to ("
                << maxX << "," << maxY << ") = " << cellCount << " cells" << std::endl;

            for (int y = minY; y <= maxY; ++y) {
                for (int x = minX; x <= maxX; ++x) {
                    blocked.insert({ x, y });
                }
            }

            processedCount++;
        }
    }

    gridPathfinder->SetBlockedCells(blocked);

    std::cout << "[PathFinding] Processed: " << processedCount << " walls" << std::endl;
    std::cout << "[PathFinding] Skipped: " << skippedCount << " walls" << std::endl;
    std::cout << "[PathFinding] Total blocked cells: " << blocked.size() << std::endl;
    std::cout << "[PathFinding] ========================================" << std::endl;
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
    if (gridPathfinder && false) { // Set to true to enable
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
