#pragma once

#include <string>
#include <memory>

namespace Uma_UI
{
	class UIManager
	{
	public:
		void RegisterScreen(const std::string& name, std::unique_ptr<UIPanel> screen);
		void ShowScreen(const std::string& name);
		void HideScreen(const std::string& name);
		void HideAllScreens();

		UIPanel* GetScreen(const std::string& name);

		void Update(float deltaTime);
		void Render();
		void HandleInput(Vector2 mousePos, bool mousePressed, bool mouseReleased);

	private:
		struct ScreenData
		{
			std::unique_ptr<UIPanel> screen;
			bool active = true;
			int sortOrder = 0;
		};

		std::unordered_map<std::string, ScreenData> screens;
		std::vector<std::string> renderOrder;

		Vector2 mousePosition{ 0, 0 };
		bool mousePressed = false;
		bool mouseReleased = false;

		bool ProcessUIElement(UIElement* element);
		void UpdateRenderOrder();
	}
}