#include "PathFindingSystem.hpp"

#include "Components/Transform.h"
#include "Components/RigidBody.h"
#include "Components/PathFinding.h"
#include "Components/Camera.h"

#include "Core/Coordinator.hpp"
#include "Systems/Graphics.hpp"

#include "Events/PlayerEvents.h"
#include "Events/InputEvents.h"

#include <GLFW/glfw3.h>

void Uma_ECS::PathFindingSystem::Init(Coordinator* c, Uma_Engine::EventSystem* es, Uma_Engine::Graphics* graphics)
{
    pCoordinator = c;
    pEventSystem = es;
    pGraphics = graphics;

    navmesh = new Uma_Navigation::DynamicNavMesh(500.f, 20.f);
    navmeshCenter = Vec2(0, 0);


    //move later
    pEventSystem->Subscribe<Uma_Engine::MouseButtonEvent>(
        [this](const Uma_Engine::MouseButtonEvent& e) {
            if (e.button != GLFW_MOUSE_BUTTON_RIGHT || e.action != GLFW_PRESS) return;

            auto& pfArray = pCoordinator->GetComponentArray<PathFinding>();

            if (pfArray.Has(playerID)) {
                auto& pf = pCoordinator->GetComponentArray<PathFinding>().GetData(playerID);
                pf.pathUpdateTimer = pf.pathUpdateInterval + 0.1f;

                pf.goal = pGraphics->ScreenToWorld(Vec2(e.x, e.y));
                pf.pathUpdateTimer = pf.pathUpdateInterval + 0.1f;
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

        if (entity == playerID) {
            /*std::cout << "[Player Update] Goal: (" << pf.goal.x << ", " << pf.goal.y
                << ") | Position: (" << tf.position.x << ", " << tf.position.y << ")" << std::endl;*/
        }

        if (entity != playerID && hasPlayer) {
            //pf.goal = playerPosition;
        }

        pf.pathUpdateTimer += dt;
        if (pf.pathUpdateTimer >= pf.pathUpdateInterval) {
            pf.path = navmesh->FindPath(tf.position, pf.goal);
            pf.pathIndex = 0;
            pf.hasValidPath = !pf.path.empty();

            if (entity == playerID) {
                std::cout << "[Player Pathfinding] From: (" << tf.position.x << ", " << tf.position.y << ")" << std::endl;
                std::cout << "[Player Pathfinding] To: (" << pf.goal.x << ", " << pf.goal.y << ")" << std::endl;
                std::cout << "[Player Pathfinding] Path waypoints: " << pf.path.size() << std::endl;
                std::cout << "[Player Pathfinding] Valid path: " << pf.hasValidPath << std::endl;

                if (pf.path.empty()) {
                    std::cout << "[Player Pathfinding] ERROR: No path found! Check navmesh coverage." << std::endl;
                }
            }

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

                /*if (entity == playerID) {
                    spd = playerSpeed;
                }
                else if (enemyArray.Has(entity)) {
                    spd = enemyArray.GetData(entity).mSpeed;
                }*/

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
