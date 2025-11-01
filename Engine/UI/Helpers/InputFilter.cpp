/*!
\file   InputFilter.cpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\brief
Implementation of InputFilter - queries UISystem for input consumption state.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

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