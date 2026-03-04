/*!
\file   PathFindingSystem.cpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Koh Kai Yang (100%)
\par    E-mail: k.kaiyang@digipen.edu
\par    DigiPen login: k.kaiyang

\brief
Implementation of updating the pathfinding component

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "PathFindingSystem.hpp"

#include "Components/Transform.h"
#include "Components/RigidBody.h"
#include "Components/PathFinding.h"

#include "Core/Coordinator.hpp"
#include "Systems/Graphics.hpp"
#include "Systems/HybridInputSystem.h"

#include "Events/InputEvents.h"
#include "Events/IMGUIEvents.h"
#include <GLFW/glfw3.h>
#include <random>

void Uma_ECS::PathFindingSystem::Init(Coordinator* c, Uma_Engine::EventSystem* es, Uma_Engine::Graphics* graphics)
{
    pCoordinator = c;
    pEventSystem = es;
    pGraphics = graphics;

    gridPathfinder = new Uma_Navigation::GridPathfinder(cellSize);
    isDirty = true;
    pEventSystem->Subscribe<Uma_Engine::CallPathFindToBake, PathFindingSystem>(
        [this](const Uma_Engine::CallPathFindToBake& e)
        {
            (void)(e);
            isDirty = true;
        }
    );

    pEventSystem->Subscribe<Uma_Engine::PlaySceneRequest, PathFindingSystem>(
        [this](const Uma_Engine::PlaySceneRequest& e)
        {
            (void)(e);
            initGoal = true;
        }
    );

    pEventSystem->Subscribe<Uma_Engine::LoadSceneRequestEvent, PathFindingSystem>(
        [this](const Uma_Engine::LoadSceneRequestEvent& e)
        {
            (void)(e);
            initGoal = true;
        }
    );
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
        if (!pCoordinator->IsActiveInHierarchy(entity))
            continue;

        if (colArray.Has(entity) && tfArray.Has(entity)) {
            const auto& collider = colArray.GetData(entity);
            const auto& tf = tfArray.GetData(entity);

            if (playerArray.Has(entity) && collider.shapes.size() > 1 && collider.shapes[1].isActive) {
                // For player, use shapes[1]
                const auto& navShape = collider.shapes[1];
                float sx = navShape.size.x * tf.scale.x;
                float sy = navShape.size.y * tf.scale.y;
                float playerRadius = (std::max)(sx, sy) * 0.5f;

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

            if (initGoal && pfArray.Has(entity)) {
                auto& pf = pfArray.GetData(entity);
                pf.goal = tf.position;
                if (playerArray.Has(entity) && collider.shapes.size() > 1 && collider.shapes[1].isActive) {
                    // For player, use shapes[1]
                    const auto& navShape = collider.shapes[1];
                    Vec2 offset{ navShape.offset.x * tf.scale.x ,navShape.offset.y * tf.scale.y };
                    pf.goal += offset;
                }
            }
        }
    }

    if (initGoal) {
        initGoal = false;
    }

    // Find player and rebuild pathfinder around them
    if (playerArray.Size() > 0) 
    {
        playerID = playerArray.GetEntity(0);
        hasPlayer = tfArray.Has(playerID) && playerArray.Has(playerID);

        if (hasPlayer) 
        {
            playerPosition = tfArray.GetData(playerID).worldPosition;

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

    //const float goalDeadZone = cellSize * 2.0f;

    // Process pathfinding for all entities
    for (auto const& entity : aEntities)
    {
        auto& tf = tfArray.GetData(entity);
        auto& pf = pfArray.GetData(entity);
        auto& rb = rbArray.GetData(entity);

        if (!pf.enabled)
            continue;

        // Stagger pathUpdateTimer for new entities so they don't all repath on the same frame
        if (staggeredEntities.find(entity) == staggeredEntities.end()) {
            static std::mt19937 rng(42);
            std::uniform_real_distribution<float> dist(0.0f, pf.pathUpdateInterval);
            pf.pathUpdateTimer = dist(rng);
            staggeredEntities.insert(entity);
        }

        bool isPlayer = playerArray.Has(entity);
        bool isEnemy = enemyArray.Has(entity);

        // Calculate current position with collider offset
        //Vec2 currentPos = tf.position;
        //if (isPlayer && colArray.Has(entity)) {
        //    const auto& collider = colArray.GetData(entity);

        //    // Player uses shapes[1] for pathfinding position
        //    if (collider.shapes.size() > 1 && collider.shapes[1].isActive) {
        //        Vec2 worldOffset = Vec2{
        //            collider.shapes[1].offset.x * tf.scale.x,
        //            collider.shapes[1].offset.y * tf.scale.y
        //        };
        //        currentPos = tf.position + worldOffset;
        //    }
        //}

        Vec2 currentPos = tf.position;
        if (colArray.Has(entity)) {
            const auto& collider = colArray.GetData(entity);
            //use shapes[1] for pathfinding position
            if (collider.shapes.size() > 1 && collider.shapes[1].isActive) {
                Vec2 worldOffset = Vec2{
                    collider.shapes[1].offset.x * tf.scale.x,
                    collider.shapes[1].offset.y * tf.scale.y
                };
                currentPos = tf.position + worldOffset;
            }
        }

        float agentRad = cellSize;
        if (colArray.Has(entity)) {
            const auto& collider = colArray.GetData(entity);

            if (isPlayer && collider.shapes.size() > 1 && collider.shapes[1].isActive) {
                // For player, use shapes[1]
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

        // Dynamic goal threshold based on movement per frame to handle different frame rates
        // At 50 units/s: 60fps moves 0.833 units/frame, 120fps moves 0.417 units/frame
        float goalThreshold = 50.0f * dt * 2.0f; // 2x max movement per frame
        if (goalThreshold < 1.0f) goalThreshold = 1.0f; // Minimum threshold

        if (isPlayer && distToGoal < goalThreshold && !pf.hasValidPath) {
            pf.reachedGoal = true;
            rb.velocity = Vec2(0, 0);
            pf.pathUpdateTimer = 0.0f;
            continue;
        }

        pf.pathUpdateTimer += dt;
        if (pf.pathUpdateTimer >= pf.pathUpdateInterval) {
            bool goalChanged = !pf.haveLastGoal || (pf.goal != pf.lastGoal);
            bool needRepath = (!pf.hasValidPath || pf.reachedGoal || goalChanged);

            // Subtract interval instead of resetting to 0 to preserve stagger offset
            pf.pathUpdateTimer -= pf.pathUpdateInterval;

            if (needRepath) {
                pf.path = gridPathfinder->FindPath(currentPos, pf.goal, agentRad);
                pf.path = gridPathfinder->SmoothPath(pf.path);
                pf.pathIndex = 0;
                pf.hasValidPath = !pf.path.empty();

                pf.lastGoal = pf.goal;
                pf.haveLastGoal = true;

                if (isPlayer) {
                    pf.reachedGoal = false;   // starting a fresh path to a goal
                }
            }
        }

        // Follow the path
        if (pf.hasValidPath && pf.pathIndex < pf.path.size()) {
            Vec2 target = pf.path[pf.pathIndex];
            Vec2 direction = target - currentPos;
            float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

            // Get speed first to calculate dynamic tolerance
            float spd = 50.0f;
            if (isPlayer) {
                spd = 50.0f;
            }
            else if (isEnemy) {
                spd = enemyArray.GetData(entity).mSpeed;
            }

            // Dynamic waypoint tolerance based on speed and dt to handle different frame rates
            // Tolerance = base tolerance + max distance entity can move in one frame
            float baseWaypointTolerance = cellSize * 0.5f;
            if (isPlayer) {
                baseWaypointTolerance = cellSize * 0.3f;
            }
            else if (isEnemy) {
                baseWaypointTolerance = cellSize * 0.6f;
            }

            // Add safety margin: max movement per frame + small buffer
            float maxFrameMovement = spd * dt * 1.2f; // 1.2x for safety
            float waypointTolerance = baseWaypointTolerance + maxFrameMovement;

            if (distance < waypointTolerance) {
                pf.pathIndex++;
                if (pf.pathIndex >= pf.path.size()) {
                    pf.reachedGoal = true;
                    pf.hasValidPath = false;
                    rb.velocity = Vec2(0, 0);
                }
            }
            else if (distance > 0.001f) {
                spd = 50.0f;
                if (isPlayer) {
                    spd = playerArray.GetData(playerID).mSpeed;
                }
                else if (isEnemy) {
                    spd = enemyArray.GetData(entity).mSpeed;
                }

                rb.velocity = Vec2(direction.x / distance * spd, direction.y / distance * spd);
            }
        }
        else {
            rb.velocity = Vec2(0, 0);

            // Use dynamic threshold for enemy goal reaching as well
            float enemyGoalThreshold = cellSize * 2.0f;
            if (isEnemy) {
                float enemySpeed = enemyArray.GetData(entity).mSpeed;
                float enemyFrameMovement = enemySpeed * dt * 2.0f;
                enemyGoalThreshold = (std::max)(enemyGoalThreshold, enemyFrameMovement);
            }

            if (isEnemy && distToGoal < enemyGoalThreshold) {
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
    staggeredEntities.clear();

    //to be removed later
    pEventSystem->UnsubscribeSystem<PathFindingSystem>();
}
