/*!
\file   ICommand.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Defines the ICommand interface, which represents a generic command used by the
Editor’s Undo/Redo system. All editor commands must implement Execute(),
Undo(), and GetDescription(), enabling consistent behavior across all user
actions recorded in the command history stack.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include <string>

namespace Uma_Editor
{
    class ICommand
    {
    public:
        virtual ~ICommand() {};
        virtual void Execute() = 0;
        virtual void Undo() = 0;
        virtual std::string GetDescription() = 0;
    };
}