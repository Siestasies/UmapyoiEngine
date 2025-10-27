#pragma once

#include <map>

#include "../Core/UIElement.h"

namespace Uma_UI
{

	enum class UIRenderMode
	{
		ScreenSpaceOverlay,
		ScreenSpaceCamera,
		WorldSpace
	};

	enum class UIScaleMode
	{
		ConstantPixelSize,
		ConstantPhysicalSize,
		ScreenSize
	};

	class UICanvas : public UIElement
	{
	public:
		UIRenderMode renderMode = UIRenderMode::ScreenSpaceOverlay;
		UIScaleMode scaleMode = UIScaleMode::ScreenSize;

		Vec2 referenceResolution{1920, 1080};
		float matchWidth = 0.5f, matchHeight = 0.5f;

		int sortingOrder = 0;

		enum class Layer
		{
			Background = 0,
			Default = 100,
			Foreground = 200,
			Overlay = 300,
			Popup = 400
		};

		void SetLayer(UIElement* element, int layer);
		void UpdateScaling(Vec2 screenSize);

		void Render() override;

	private:
		std::map<int, std::vector<UIElement*>> layeredElements;
		float scaleFactor = 1.f;
	};
}