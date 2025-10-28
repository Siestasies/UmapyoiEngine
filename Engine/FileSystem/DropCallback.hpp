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

} // namespace Uma_Engine