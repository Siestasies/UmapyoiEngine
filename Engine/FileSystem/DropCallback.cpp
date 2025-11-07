#include "DropCallback.hpp"

namespace Uma_Engine
{
    std::vector<std::string> FileDropHandler::aDroppedFiles;

    void FileDropHandler::DropCallback(GLFWwindow* window, int count, const char** paths)
    {
        (void)window;

        for (int i = 0; i < count; i++)
        {
            aDroppedFiles.push_back(paths[i]);
        }
    }
}