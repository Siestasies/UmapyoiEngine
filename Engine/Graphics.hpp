#pragma once
#include <GLFW/glfw3.h>
#include <vector>

namespace UmapyoiEngine
{
    struct Color
    {
        float r, g, b, a;
        Color(float red = 1.0f, float green = 1.0f, float blue = 1.0f, float alpha = 1.0f)
            : r(red), g(green), b(blue), a(alpha) {}
        Color(int red, int green, int blue, int alpha = 255)
            : r(red / 255.0f), g(green / 255.0f), b(blue / 255.0f), a(alpha / 255.0f) {}
    };

    struct Transform
    {
        float x, y;
        float rotation;
        float scale;

        Transform(float posX = 0.0f, float posY = 0.0f, float rot = 0.0f, float s = 1.0f)
            : x(posX), y(posY), rotation(rot), scale(s) {}
    };

    class Graphics
    {
    private:
        static bool sInitialized;
        static int sWindowWidth;
        static int sWindowHeight;

        // OpenGL objects
        static unsigned int mVAO;
        static unsigned int mVBO;
        static unsigned int mEBO;
        static unsigned int mShaderProgram;

    public:
        // Core functions
        static bool Initialize(int windowWidth, int windowHeight);
        static void Shutdown();
        static void ClearBackground(const Color& color = Color(0.0f, 0.0f, 0.0f));
    };
}