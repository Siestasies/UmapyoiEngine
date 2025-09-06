#pragma once
#include <GLFW/glfw3.h>
#include <string>

namespace UmapyoiEngine
{
    
    class Window
    {
    public:
        Window(int width, int height, const std::string& title);
        ~Window();
        
        bool Initialize();
        void Update();
        void Shutdown();
        
        bool ShouldClose() const;
        int GetWidth() const { return m_Width; }
        int GetHeight() const { return m_Height; }
        
        // For checking if ESC was pressed
        bool IsKeyPressed(int key) const;
        
    private:
        GLFWwindow* m_Window;
        int m_Width;
        int m_Height;
        std::string m_Title;
        bool m_Initialized;
    };
    
}