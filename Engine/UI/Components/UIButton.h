#pragma once

#include "UIElement.h"
#include <functional>

namespace Uma_UI
{
    class UIButton : public UIElement
    {
    public:
        Colour normalColor{ 1.f, 1.f, 1.f, 1.f };
        Colour hoverColor{ 0.9f, 0.9f, 0.9f, 1.f };
        Colour pressedColor{ 0.7f, 0.7f, 0.7f, 1.f };

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