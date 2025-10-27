#include "UICanvas.hpp"
#include <algorithm>

namespace Uma_UI
{
	void UICanvas::SetLayer(UIElement* element, int layer)
	{
		layeredElements[layer].push_back(element);
	}

	void UICanvas::UpdateScaling(Vec2 screenSize)
	{
		if (scaleMode == UIScaleMode::ConstantPixelSize)
		{
			scaleFactor = 1.f;
		}
		else if (scaleMode == UIScaleMode::ScreenSize)
		{
			float widthScale = screenSize.x / referenceResolution.x;
			float heightScale = screenSize.x / referenceResolution.x;
			scaleFactor = widthScale * (1.f - matchWidth) + heightScale * matchHeight;
		}
	}

	void UICanvas::Render()
	{
		// Interesting...
		for (auto& [layer, elements] : layeredElements)
		{
			for (auto* element : elements)
			{
				if (element && element->visible)
				{
					element->Render();
				}
			}
		}
	}
}