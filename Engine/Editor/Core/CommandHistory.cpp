#include "CommandHistory.h"

namespace Uma_Editor
{
    void CommandHistory::ExecuteCommand(std::unique_ptr<ICommand> cmd)
    {
        cmd->Execute();
        undoStack.push_back(std::move(cmd));
        redoStack.clear();

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