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