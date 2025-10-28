#include "../Components/UIButton.h"

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
        Colour currentColor = normalColour;
        if (isPressed) currentColor = pressedColour;
        else if (isHovered) currentColor = hoverColour;
        // TODO: Pixel-space rendering
        UIElement::Render();
    }
}