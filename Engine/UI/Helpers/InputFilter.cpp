#include "../Helpers/InputFilter.h"
#include "../Systems/UISystem.h"

namespace Uma_UI
{
    // Static member definition
    UISystem* InputFilter::pUI = nullptr;

    bool InputFilter::ShouldBlockMouseInput()
    {
        if (!pUI)
        {
            return false;
        }
        
        return pUI->IsMouseConsumedByUI();
    }

    bool InputFilter::IsMouseOverUI()
    {
        if (!pUI)
        {
            return false;
        }
        
        return pUI->IsUIHovered();
    }

    bool InputFilter::ShouldBlockKeyboardInput()
    {
        // For now, UI doesn't consume keyboard input
        return false;
    }
}