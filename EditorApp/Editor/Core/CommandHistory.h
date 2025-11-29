/*!
\file   CommandHistory.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Declares the CommandHistory class, which manages the Undo/Redo system used
by the Uma Editor. The class stores executed ICommand objects in a bounded
history buffer, allowing users to undo and redo editor actions. The history
is maintained with two stacks—undo and redo—and enforces a maximum capacity
to control memory usage.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "Command.h"
#include <vector>
#include <memory>

namespace Uma_Editor
{
    /**
     * \class CommandHistory
     * \brief Manages a stack-based Undo/Redo history of editor commands.
     *
     * This class stores executed ICommand instances in two internal stacks:
     * - An undo stack for reversing operations.
     * - A redo stack for reapplying previously undone actions.
     *
     * When ExecuteCommand() is called, the command is executed immediately,
     * pushed onto the undo stack, and the redo stack is cleared.
     *
     * Undo() moves the last command from undo stack → redo stack after calling Undo().
     * Redo() performs the inverse: redo stack → undo stack while calling Execute().
     *
     * The history preserves a maximum number of commands, removing the oldest entry
     * when the limit is exceeded.
     */
    class CommandHistory
    {
    private:
        std::vector<std::unique_ptr<ICommand>> undoStack;  //!< Commands available for undoing
        std::vector<std::unique_ptr<ICommand>> redoStack;  //!< Commands available for redoing
        size_t maxStackSize = 30;                          //!< Maximum history buffer size

    public:
        /**
         * \brief Executes a command and pushes it onto the undo stack.
         * \param cmd A heap-allocated ICommand instance.
         */
        void ExecuteCommand(std::unique_ptr<ICommand> cmd);

        /**
         * \brief Undoes the most recent command in the undo stack.
         * Moves the undone command into the redo stack.
         */
        void Undo();

        /**
         * \brief Redoes the most recent undone command.
         * Moves the command back into the undo stack.
         */
        void Redo();

        /**
         * \brief Clears all Undo and Redo history.
         */
        void Clear();
    };
}
