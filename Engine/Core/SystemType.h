/*!
\file   SystemType.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

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

    // Base system interface - only requires basic lifecycle methods
    class ISystem
    {
    public:
        virtual ~ISystem() = default;

        virtual void Init() = 0;

        virtual void Update(float dt) = 0;

        virtual void Shutdown() = 0;

        inline void SetSystemManager(SystemManager* sm) { pSystemManager = sm; }

    protected:
        SystemManager* pSystemManager = nullptr;
    };
}