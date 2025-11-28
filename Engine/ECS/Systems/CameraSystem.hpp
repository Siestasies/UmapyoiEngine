/*!
\file   CameraSystem.hpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Defines camera management system that controls camera positioning and player following mechanics.

Inherits from ECSSystem and maintains reference to Coordinator for component array access.
Provides initialization with Coordinator pointer and per-frame Update method.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "../Core/System.hpp"
#include "../Core/Coordinator.hpp"

namespace Uma_ECS
{
    class CameraSystem : public ECSSystem
    {
    public:

        /*!
        \brief Initializes the CameraSystem with a reference to the Coordinator.
        \param c Pointer to the main ECS Coordinator to access components and entities.
        */
        inline void Init(Coordinator* c)
        {
            pCoordinator = c;
        }

        /*!
        \brief Updates the camera logic for the current frame.

        Performs the following operations:
        1. Checks if the main camera is active.
        2. Snaps the camera position to the player if 'followPlayer' is enabled.
        3. Calculates and applies screen shake offsets if a shake event is active.

        \param dt Delta time (time step) for the current frame, used for shake timers.
        */
        void Update(float dt);


    private:

        Coordinator* pCoordinator = nullptr;
    };
}