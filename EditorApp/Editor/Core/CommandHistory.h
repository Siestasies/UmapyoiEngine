#pragma once

#include "Command.h"
#include <vector>
#include <stack>
#include <memory>

namespace Uma_Editor
{
    class CommandHistory
    {
    private:
        std::vector<std::unique_ptr<ICommand>> undoStack;
        std::vector<std::unique_ptr<ICommand>> redoStack;
        size_t maxStackSize = 30;

    public:

        void ExecuteCommand(std::unique_ptr<ICommand> cmd);
        void Undo();
        void Redo();
        void Clear();
    };
}
