/*!
\file   CollisionSystem.hpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Defines collision detection and resolution system using spatial hashing with AABB primitives.

Unity-inspired approach with contact normals, velocity projection, and purpose-based resolution.
Provides layer-based collision filtering through bitmask operations on Collider components.
Cell struct and CellHash functor enable grid-based spatial partitioning with configurable CELL_SIZE constant.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "Core/System.hpp"
#include "Core/Coordinator.hpp"
#include "Components/Collider.h"

#include "Core/EventSystem.h"

#include <unordered_set>

const float CELL_SIZE = 100.0f; // Tune based on your game world

namespace Uma_ECS
{
    struct Cell
    {
        int x, y;

        bool operator==(const Cell& other) const
        {
            return x == other.x && y == other.y;
        }
    };

    // Hash function for spatial grid
    struct CellHash
    {
        std::size_t operator()(const Cell& c) const
        {
            return (std::hash<int>()(c.x) ^ (std::hash<int>()(c.y) << 1));
        }
    };

    struct Transform;
    struct Collider;
    struct RigidBody;

    class CollisionSystem : public ECSSystem
    {
    public:
        inline void Init(Coordinator* c, Uma_Engine::EventSystem* e) { gCoordinator = c; pEventSystem = e; }

        void Update(float dt);

    private:
        // Bounding box update
        void UpdateBoundingBoxes();

        // Collision detection and resolution
        void UpdateCollision(float dt);

        void CheckEntityPairCollision(
            Entity e1, Entity e2,
            ComponentArray<Transform>& tfArray,
            ComponentArray<Collider>& cArray,
            ComponentArray<RigidBody>& rbArray,
            float dt);

        // Purpose-based collision filtering
        bool ShouldPurposesCollide(ColliderPurpose p1, ColliderPurpose p2);

        // Unity-style collision handling
        void HandleShapeCollision(
            Entity e1, Entity e2,
            Transform& tf1, Transform& tf2,
            RigidBody* rb1, RigidBody* rb2,
            const BoundingBox& box1, const BoundingBox& box2,
            ColliderPurpose purpose1, ColliderPurpose purpose2);

        // Unity-style AABB resolution with contact normals
        void ResolveAABBCollision(
            Transform& tf1, Transform& tf2,
            const BoundingBox& box1, const BoundingBox& box2,
            bool e1CanMove, bool e2CanMove,
            RigidBody* rb1, RigidBody* rb2);

        // Helper functions
        Vec2 GetCollisionNormal(const BoundingBox& box1, const BoundingBox& box2);

        inline int WorldToCell(float coord)
        {
            return static_cast<int>(std::floor(coord / CELL_SIZE));
        }

        void InsertIntoGrid(
            std::unordered_map<Cell, std::vector<Entity>, CellHash>& grid,
            Entity e,
            const BoundingBox& box);

        // tracking the entity pairs that are currently colliding
        struct EntityPair
        {
            Entity entityA;
            Entity entityB;

            EntityPair(Entity a, Entity b)
                : entityA((a < b) ? a : b)
                , entityB((a < b) ? b : a)
            {
            }

            bool operator==(const EntityPair& other) const
            {
                return entityA == other.entityA && entityB == other.entityB;
            }
        };

        struct EntityPairHash
        {
            std::size_t operator()(const EntityPair& p) const
            {
                return std::hash<Uma_ECS::Entity>()(p.entityA) ^
                    (std::hash<Uma_ECS::Entity>()(p.entityB) << 1);
            }
        };

        // AABB intersection test
        bool CollisionIntersection_RectRect_Static(
            const BoundingBox& lhs,
            const BoundingBox& rhs);

        Coordinator* gCoordinator = nullptr;
        Uma_Engine::EventSystem* pEventSystem = nullptr;

        std::unordered_set<EntityPair, EntityPairHash> currentCollisions;
        std::unordered_set<EntityPair, EntityPairHash> previousCollisions;
    };
}