#include "PathFindingSystem.hpp"

#include "../Components/Transform.h"
#include "../Components/RigidBody.h"
#include "../Components/PathFinding.h"

#include "../Core/Coordinator.hpp"

#include "Events/PlayerEvents.h"

void Uma_ECS::PathFindingSystem::Init(Coordinator* c, Uma_Engine::EventSystem* es)
{
    pCoordinator = c;
    pEventSystem = es;

    navmesh = new Uma_Navigation::DynamicNavMesh(150.f, 20.f);
    navmeshCenter = Vec2(0, 0);

    //playerID = pCoordinator->GetComponentArray<Player>().GetEntity(0);

    //temp holder please update when you have correct event
    //need to convert screen space to world space
    pEventSystem->Subscribe<Uma_Engine::PlayerMoveEvent>(
        [this](const Uma_Engine::PlayerMoveEvent& e) {
            auto& pf = pCoordinator->GetComponentArray<PathFinding>().GetData(e.playerId);
            //when i have the correct event for position
            
            //force the goal to update
            pf.pathUpdateTimer = pf.pathUpdateInterval;
        });
}

void Uma_ECS::PathFindingSystem::Update(float dt)
{
    auto& tfArray = pCoordinator->GetComponentArray<Transform>();
    auto& rbArray = pCoordinator->GetComponentArray<RigidBody>();
    auto& pfArray = pCoordinator->GetComponentArray<PathFinding>();
    auto& playerArray = pCoordinator->GetComponentArray<Player>();
    auto& enemyArray = pCoordinator->GetComponentArray<Enemy>();

    Entity playerID = 0;
    Vec2 playerPosition;
    float playerSpeed = 100.0f;
    bool hasPlayer = false;

    if (playerArray.Size() > 0) {
        playerID = playerArray.GetEntity(0);
        hasPlayer = tfArray.Has(playerID) && playerArray.Has(playerID);

        if (hasPlayer) {
            playerPosition = tfArray.GetData(playerID).position;
            playerSpeed = playerArray.GetData(playerID).mSpeed;
            navmeshCenter = playerPosition;
        }
    }

    //capping update so the expensive checks are limited
    navmeshUpdateTimer += dt;
    if (navmeshUpdateTimer >= 0.1f) {
        navmesh->Update(navmeshCenter);
        navmeshUpdateTimer = 0.0f;
    }

    for (auto const& entity : aEntities)
    {
        auto& tf = tfArray.GetData(entity);
        auto& pf = pfArray.GetData(entity);
        auto& rb = rbArray.GetData(entity);

        if (entity == playerID && hasPlayer) {
            continue;
        }

        if (entity != playerID && hasPlayer) {
            pf.goal = playerPosition;
        }

        pf.pathUpdateTimer += dt;
        if (pf.pathUpdateTimer >= pf.pathUpdateInterval) {
            pf.path = navmesh->FindPath(tf.position, pf.goal);
            pf.pathIndex = 0;
            pf.hasValidPath = !pf.path.empty();
            pf.pathUpdateTimer = 0.0f;
            pf.reachedGoal = false;
        }

        if (pf.hasValidPath && pf.pathIndex < pf.path.size()) {
            Vec2 target = pf.path[pf.pathIndex];
            Vec2 direction = target - tf.position;
            
            //movement of the entites towards way point
            float distance = Uma_Math::magnitude(direction);

            if (distance < 2.0f) {
                pf.pathIndex++;
                if (pf.pathIndex >= pf.path.size()) {
                    pf.reachedGoal = true;
                    pf.hasValidPath = false;
                    rb.velocity = Vec2(0, 0);
                }
            }
            else if(distance > 0.001f) {
                float spd = 50.0f;  // Default

                if (entity == playerID) {
                    spd = playerSpeed;
                }
                else if (enemyArray.Has(entity)) {
                    spd = enemyArray.GetData(entity).mSpeed;
                }

                rb.velocity = direction * (1.0f / distance) * spd;
            }
        }
        else {
            rb.velocity = Vec2(0, 0);
        }
    }
}

void Uma_ECS::PathFindingSystem::Shutdown()
{
    delete navmesh;
    navmesh = nullptr;
}

void Uma_ECS::PathFindingSystem::LoadTiles(const std::vector<Uma_Navigation::Tile>& tiles)
{
    navmesh->BuildSpatialHash(tiles);
    navmesh->Update(navmeshCenter);
}
