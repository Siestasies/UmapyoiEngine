#pragma once

#include "../Core/UIElement.h"

namespace Uma_UI
{
	class UIPanel : public UIElement
	{
	public:
		Colour backgroundColour{ 0.2f, 0.2f, 0.2f, 0.8f };

		void Render() override;
	};
}