#include "UIManager.hpp"

namespace Uma_UI
{
#include "Engine/UI/Core/UIManager.hpp"
#include "Engine/UI/Components/UIButton.hpp"
#include <algorithm>

	void UIManager::RegisterScreen(const std::string& name, std::unique_ptr<UIPanel> screen)
	{
		ScreenData data;
		data.screen = std::move(screen);
		data.active = false;
		data.sortOrder = static_cast<int>(screens.size());
		screens[name] = std::move(data);
		UpdateRenderOrder();
	}

	void UIManager::ShowScreen(const std::string& name)
	{
		auto it = screens.find(name);
		if (it != screens.end()) it->second.active = true;
	}

	void UIManager::HideScreen(const std::string& name)
	{
		auto it = screens.find(name);
		if (it != screens.end()) it->second.active = false;
	}

	void UIManager::HideAllScreens()
	{
		for (auto& [name, data] : screens) data.active = false;
	}

	UIPanel* UIManager::GetScreen(const std::string& name)
	{
		auto it = screens.find(name);
		return it != screens.end() ? it->second.screen.get() : nullptr;
	}

	void UIManager::Update(float deltaTime)
	{
		for (const auto& screenName : renderOrder)
		{
			auto& data = screens[screenName];
			if (data.active && data.screen) data.screen->Update(deltaTime);
		}

		if (mousePressed || mouseReleased)
		{
			for (auto it = renderOrder.rbegin(); it != renderOrder.rend(); ++it)
			{
				auto& data = screens[*it];
				if (data.active && data.screen && ProcessUIElement(data.screen.get())) break;
			}
		}
	}

	void UIManager::Render()
	{
		for (const auto& screenName : renderOrder)
		{
			auto& data = screens[screenName];
			if (data.active && data.screen) data.screen->Render();
		}
	}

	void UIManager::HandleInput(Vector2 mousePos, bool pressed, bool released)
	{
		mousePosition = mousePos;
		mousePressed = pressed;
		mouseReleased = released;
	}

	bool UIManager::ProcessUIElement(UIElement* element)
	{
		if (!element->enabled || !element->visible) return false;

		if (auto* button = dynamic_cast<UIButton*>(element))
		{
			Rect rect = button->GetRect();
			bool wasHovered = button->isHovered;
			button->isHovered = rect.Contains(mousePosition);

			if (button->isHovered && !wasHovered) button->OnPointerEnter();
			else if (!button->isHovered && wasHovered) button->OnPointerExit();

			if (button->isHovered && mousePressed)
			{
				button->OnPointerDown();
				return true;
			}

			if (mouseReleased)
			{
				button->OnPointerUp();
				if (button->isHovered) return true;
			}
		}

		for (auto& child : element->children)
		{
			if (ProcessUIElement(child.get())) return true;
		}

		return false;
	}

	void UIManager::UpdateRenderOrder()
	{
		renderOrder.clear();
		for (const auto& [name, data] : screens) renderOrder.push_back(name);
		std::sort(renderOrder.begin(), renderOrder.end(), [this](const std::string& a, const std::string& b){return screens[a].sortOrder < screens[b].sortOrder;});
	}
}