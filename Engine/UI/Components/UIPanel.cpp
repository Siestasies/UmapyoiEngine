#include "UIPanel.h"

namespace Uma_UI
{
    void UIPanel::Render()
    {
        if (!visible) return;
        Rect rect = GetRect();
        // TODO: Pixel-space rendering
        UIElement::Render();
    }
}