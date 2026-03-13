/*!
\file   SystemType.h
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A
\par    Software Engineering Project 4

\author Shahir Rasid (100%)
\par    E-mail: b.muhammadshahir@digipen.edu
\par    DigiPen login: b.muhammadshahir

\brief
This file implements the definition for a base class of system in that
anything that wants to be a system should inherit from this class.
Systems must be registered to system manager to be used.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/
#pragma once

namespace Uma_Engine
{
    // foward declare of System Manager
    class SystemManager;

    /*!
     * \class ISystem
     * \brief Base interface for all engine systems providing lifecycle methods.
     */
    class ISystem
    {
    public:
        virtual ~ISystem() = default;

        /*!
         * \brief Initializes the system. Called once after registration.
         */
        virtual void Init() = 0;

        /*!
         * \brief Updates the system each frame.
         * \param dt Delta time in seconds since the last frame.
         */
        virtual void Update(float dt) = 0;

        /*!
         * \brief Shuts down the system, releasing any resources.
         */
        virtual void Shutdown() = 0;

        /*!
         * \brief Sets the parent system manager reference.
         * \param sm Pointer to the owning SystemManager.
         */
        inline void SetSystemManager(SystemManager* sm) { pSystemManager = sm; }

    protected:
        SystemManager* pSystemManager = nullptr;
    };
}