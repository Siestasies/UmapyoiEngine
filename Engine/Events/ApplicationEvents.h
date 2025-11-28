/*!
\file   SceneEvents.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Defines events related to scene lifecycle management within the engine.

Includes notifications for scene changes, successful loads, and unloads,
each carrying the associated scene name and assigned appropriate priority.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "EventType.h"

namespace Uma_Engine
{
    class ApplicationQuitRequest : public Event
    {
    public:
        ApplicationQuitRequest(){ priority = Priority::High; }
    };

   
}