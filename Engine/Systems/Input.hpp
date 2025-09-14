#pragma once // Use pragma once for header guards

#include <GLFW/glfw3.h>
#include <array>

namespace Uma_Engine
{
	class Input
	{
	private:
		// Static members
		static std::array<bool, GLFW_KEY_LAST + 1> sKeys;
		static std::array<bool, GLFW_KEY_LAST + 1> sKeysPrevFrame;
		static std::array<bool, GLFW_MOUSE_BUTTON_LAST + 1> sMouseButtons;
		static std::array<bool, GLFW_MOUSE_BUTTON_LAST + 1> sMouseButtonsPrevFrame;
		static double sMouseX;
		static double sMouseY;

		// GLFW Callbacks
		static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
		static void CursorPositionCallback(GLFWwindow* window, double xpos, double ypos);

		// Get key name
		static const char* GetKeyName(int key);

	public:
		// Initialize
		static void Initialize(GLFWwindow* window);
		// Update
		static void Update();

		// Keyboard button functions
		static bool KeyDown(int key);
		static bool KeyPressed(int key);
		static bool KeyReleased(int key);

		// Mouse button functions
		static bool MouseButtonDown(int button);
		static bool MouseButtonPressed(int button);
		static bool MouseButtonReleased(int button);

		// Mouse position functions
		static void GetMousePosition(double& x, double& y);
		static double GetMouseX();
		static double GetMouseY();
	};
}