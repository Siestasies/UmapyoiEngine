#include "UIButton.h"

namespace Uma_UI
{
    void UIButton::OnPointerEnter() { isHovered = true; }
    void UIButton::OnPointerExit() { isHovered = false; isPressed = false; }
    void UIButton::OnPointerDown() { if (isHovered) isPressed = true; }
    void UIButton::OnPointerUp()
    {
        if (isHovered && isPressed && onClick) onClick();
        isPressed = false;
    }

    void UIButton::Render()
    {
        if (!visible) return;
        if (!layoutDirty) rect = GetRect();
        Colour currentColor = normalColor;
        if (isPressed) currentColor = pressedColor;
        else if (isHovered) currentColor = hoverColor;
        // TODO: Pixel-space rendering
        UIElement::Render();
    }
}