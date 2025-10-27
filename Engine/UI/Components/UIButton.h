#pragma once

#include <functional>

#include "../Core/UIElement.h"

namespace Uma_UI
{
    class UIButton : public UIElement
    {
    public:
        Colour normalColour{ 1.f, 1.f, 1.f, 1.f };
        Colour hoverColour{ 0.9f, 0.9f, 0.9f, 1.f };
        Colour pressedColour{ 0.7f, 0.7f, 0.7f, 1.f };

        std::function<void()> onClick;

        bool isHovered = false;
        bool isPressed = false;

        void OnPointerEnter();
        void OnPointerExit();
        void OnPointerDown();
        void OnPointerUp();

        void Render() override;
    };
}