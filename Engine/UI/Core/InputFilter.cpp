/*!
\file   InputFilter.cpp
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A
\par    Software Engineering Project 4

\author Jedrek Lee Jing Wei (100%)
\par    E-mail: jedrekjingwei.lee@digipen.edu
\par    DigiPen login: jedrekjingwei.lee

\brief
Implementation of the InputFilter helper class.

This file provides the concrete implementation for querying UI input consumption
state to prevent game input processing when interacting with UI elements.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "../Core/InputFilter.h"
#include "../Systems/UISystem.h"

namespace Uma_UI
{
    UISystem* InputFilter::pUI = nullptr;

    /*!
     * \brief Checks if mouse input should be blocked from the game.
     * \return True if UI is consuming mouse input this frame.
     */
    bool InputFilter::ShouldBlockMouseInput()
    {
        if (!pUI)
        {
            return false;
        }
        return pUI->IsMouseConsumedByUI();
    }

    /*!
     * \brief Checks if the mouse cursor is currently over any UI element.
     * \return True if a UI element is hovered.
     */
    bool InputFilter::IsMouseOverUI()
    {
        if (!pUI)
        {
            return false;
        }
        return pUI->IsUIHovered();
    }

    /*!
     * \brief Checks if keyboard input should be blocked from the game.
     * \return True if UI text input is active.
     */
    bool InputFilter::ShouldBlockKeyboardInput()
    {
        return false;
    }
}