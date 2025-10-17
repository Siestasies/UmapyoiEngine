/*!
\file   SceneType.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Shahir Rasid (100%)
\par    E-mail: b.muhammadshahir@digipen.edu
\par    DigiPen login: b.muhammadshahir

\brief
This file implements the definition for a base class of scene in that
anything that wants to be a system should inherit from this class.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/
#pragma once

namespace Uma_Engine
{
    class Scene
    {
        public:
            virtual void OnLoad() = 0;
            virtual void OnUnload() = 0;
            virtual void Update(float dt) = 0;
            virtual void Render() = 0;

            virtual ~Scene() = default;
    };
}