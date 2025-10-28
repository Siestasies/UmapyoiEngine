#pragma once
#include <vector>
#include <string>
#include <GLFW/glfw3.h>

namespace Uma_Engine
{

class FileBrowser;

class FileDropHandler
{
public:
    static void DropCallback(GLFWwindow* window, int count, const char** paths);

private:
    static std::vector<std::string> aDroppedFiles;

    friend class FileBrowser;
};

std::vector<std::string> FileDropHandler::aDroppedFiles;

void FileDropHandler::DropCallback(GLFWwindow* window, int count, const char** paths) 
{
    for (int i = 0; i < count; i++) {
        aDroppedFiles.push_back(paths[i]);
    }
}

} // namespace Uma_Engine