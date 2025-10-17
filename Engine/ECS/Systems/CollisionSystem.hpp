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
Defines collision detection and resolution system using spatial hashing with AABB (axis-aligned bounding box) primitives.

Provides layer-based collision filtering through bitmask operations on Collider components.
Cell struct and CellHash functor enable grid-based spatial partitioning with configurable CELL_SIZE constant.
Exposes helper methods for layer management (ShouldCollide, IsInLayer, SetLayer, AddMask) and internal methods
for grid insertion, static/dynamic AABB intersection tests, and penetration-based collision resolution.
Maintains Coordinator reference for component array access.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "../Core/System.hpp"
#include "../Core/Coordinator.hpp"

#include "Components/Collider.h"

const float CELL_SIZE = 50.0f; // tune this based on your game world

namespace Uma_ECS
{
    struct Cell {
        int x, y;

        bool operator==(const Cell& other) const {
            return x == other.x && y == other.y;
        }
    };

    // custom hash for unordered_map
    // this is to make cell into the hash key
    // so that the unorderedmap can be accessed using this hash key
    struct CellHash {
        std::size_t operator()(const Cell& c) const {
            // mix x and y into one hash
            return (std::hash<int>()(c.x) ^ (std::hash<int>()(c.y) << 1));
        }
    };

    struct Transform;
    struct Collider;
    struct RigidBody;
    enum class ColliderPurpose;
    struct BoundingBox;

    class CollisionSystem : public ECSSystem
    {
    public:

        inline void Init(Coordinator* c) { gCoordinator = c; }

        void Update(float dt);

        //inline bool ShouldCollide(const Collider& lhs, const Collider& rhs)
        //{
        //    // check if they shd collider for both of their layer and collider mask
        //    return (lhs.layer & rhs.colliderMask) && (lhs.colliderMask & rhs.layer);
        //}

        //inline bool IsInLayer(const Collider& col, CollisionLayer layer) 
        //{
        //    return (col.layer & layer) != 0;
        //}

        //inline void SetLayer(Collider& col, CollisionLayer layer) 
        //{
        //    col.layer = layer;
        //}

        //inline void AddMask(Collider& col, CollisionLayer layer) 
        //{
        //    col.colliderMask |= layer;
        //}




    private:

        void UpdateBoundingBoxes();

        void UpdateCollision(float dt);

        bool ShouldShapesCollide(
            const ColliderPurpose& shape1Purpose,
            const ColliderPurpose& shape2Purpose);

        void HandleShapeCollision(
            Entity e1, Entity e2,
            Transform& tf1, Transform& tf2,
            RigidBody* rb1, RigidBody* rb2,
            const BoundingBox& box1, const BoundingBox& box2,
            ColliderPurpose purpose1, ColliderPurpose purpose2);

        Vec2 GetCollisionNormal(
            const BoundingBox& box1,
            const BoundingBox& box2);

        void ResolveAABBDynamicCollision(
            Transform& tf1, Transform& tf2,
            const BoundingBox& box1, const BoundingBox& box2,
            bool e1CanMove, bool e2CanMove);

        void ResolveAABBStaticCollision(Transform& lhsTransform, const BoundingBox& lhsBound,
            Transform& rhsTransform, const BoundingBox& rhsBound);

        inline int WorldToCell(float coord) {
            return static_cast<int>(std::floor(coord / CELL_SIZE));
        }

        void InsertIntoGrid(
            std::unordered_map<Cell, std::vector<Entity>, CellHash>& grid,
            Entity e,
            const BoundingBox& box);

        // AABB Collsion
        bool CollisionIntersection_RectRect_Static(const BoundingBox& lhs, const BoundingBox& rhs);

        bool CollisionIntersection_RectRect_Dynamic(const BoundingBox& lhs,
            const Vec2& vel1,
            const BoundingBox& rhs,
            const Vec2& vel2,
            float& firstTimeOfCollision,
            float dt);

        inline bool CollisionIntersection_RectRect(const BoundingBox& lhs,
            const Vec2& vel1,
            const BoundingBox& rhs,
            const Vec2& vel2,
            float& firstTimeOfCollision,
            float dt)
        {
            //Step 1
            bool staticCollision = false;
            staticCollision = CollisionIntersection_RectRect_Static(lhs, rhs);
            if (staticCollision)
            {
                return true;
            }

            //Step 2 until 5
            return CollisionIntersection_RectRect_Dynamic(lhs, vel1, rhs, vel2, firstTimeOfCollision, dt);
        }

        Coordinator* gCoordinator = nullptr;
    };
}