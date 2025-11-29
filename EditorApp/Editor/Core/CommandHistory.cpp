/*!
\file   CommandHistory.cpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Implements the CommandHistory class, which manages execution, undo,
and redo operations for editor commands. The system maintains two stacks:
one for undo operations and one for redo operations, automatically
handling cleanup, stack limits, and ownership of ICommand instances.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "CommandHistory.h"

namespace Uma_Editor
{
    void CommandHistory::ExecuteCommand(std::unique_ptr<ICommand> cmd)
    {
        cmd->Execute();
        undoStack.push_back(std::move(cmd));
        redoStack.clear();

        // Trim oldest entries if exceeding storage limits
        if (undoStack.size() > maxStackSize)
        {
            undoStack.erase(undoStack.begin());
        }
    }

    void CommandHistory::Undo()
    {
        if (undoStack.empty()) return;

        std::unique_ptr<ICommand> cmd = std::move(undoStack.back());
        undoStack.pop_back();
        cmd->Undo();
        redoStack.push_back(std::move(cmd));
    }

    void CommandHistory::Redo()
    {
        if (redoStack.empty()) return;

        std::unique_ptr<ICommand> cmd = std::move(redoStack.back());
        redoStack.pop_back();
        cmd->Execute();
        undoStack.push_back(std::move(cmd));
    }

    void CommandHistory::Clear()
    {
        undoStack.clear();
        redoStack.clear();
    }
}
