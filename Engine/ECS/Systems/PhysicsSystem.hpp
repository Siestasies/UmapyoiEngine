/*!
\file   PhysicsSystem.hpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Defines the PhysicsSystem responsible for rigid-body physics updates.
This includes velocity integration, position updates, force application,
and interpolation support during fixed-timestep simulation.

The system inherits from ECSSystem and operates on entities that contain
both Transform and RigidBody components. It provides initialization using
a Coordinator reference, per-frame Update for physics calculations, and
debug logging support through PrintLog().

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "../Core/System.hpp"
#include "../Core/Coordinator.hpp"

namespace Uma_ECS
{
    struct RaycastHit {
        bool hit;
        Entity entity;
        Vec2 point;      // collision point
        Vec2 normal;     // surface normal
        float distance;
        LayerMask colliderLayer;
    };

    class PhysicsSystem : public ECSSystem
    {
    public:

        /*!
        \brief Initializes the physics system with a Coordinator reference.
        \param c Pointer to the ECS Coordinator.
        */
        inline void Init(Coordinator* c) { gCoordinator = c; }

        /*!
        \brief Updates physics simulation for all rigid-body entities.
        \param dt Delta time in seconds since last frame.
        */
        void Update(float dt);

        /*!
        \brief Saves previous positions of all physics entities before the fixed-timestep update.
        */
        void SavePrevPos();

        /*!
        \brief Applies velocities to positions for all physics entities.
        \param dt Delta time in seconds for velocity integration.
        */
        void ApplyVelocity(float dt);

        /*!
        \brief Prints debug log information for all physics entities.
        */
        void PrintLog();

        /*!
        \brief Applies an instantaneous force to a rigid-body entity.
        \param entity Entity to apply force to.
        \param pos World-space position where the force is applied.
        \param dir Direction vector of the force.
        \param force Magnitude of the force.
        \param rotation Rotational force to apply in degrees.
        */
        void AddForce(Entity entity, Vec2 pos, Vec2 dir, float force, float rotation);

        /*!
        \brief Finds all entities within a circular area.
        \param center World-space center of the circle.
        \param radius Radius of the overlap circle.
        \return Set of entities overlapping the circle.
        */
        std::unordered_set<Entity> OverlapCircle(Vec2 center, float radius);

        /*!
        \brief Checks whether two entities have an unobstructed line of sight.
        \param lhs First entity.
        \param rhs Second entity.
        \return True if there is a clear line of sight between the two entities.
        */
        bool LineOfSight(Entity lhs, Entity rhs);

        /*!
        \brief Casts a ray from an origin in a given direction and returns hit information.
        \param origin World-space starting point of the ray.
        \param dir Normalized direction vector of the ray.
        \param maxDist Maximum distance the ray can travel.
        \return RaycastHit containing collision point, normal, distance, and entity hit.
        */
        RaycastHit RayCast(Vec2 origin, Vec2 dir, float maxDist);

    private:

        Coordinator* gCoordinator = nullptr;
    };
}