/*!
\file   InputFilter.h
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A
\par    Software Engineering Project 4

\author Jedrek Lee Jing Wei (100%)
\par    E-mail: jedrekjingwei.lee@digipen.edu
\par    DigiPen login: jedrekjingwei.lee

\brief
Declares the InputFilter helper class for UI input consumption queries.

This header provides a static interface to check whether UI elements are
currently consuming input, preventing game actions from triggering when
interacting with UI.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

namespace Uma_UI
{
    class UISystem;

    /*!
     * \class InputFilter
     * \brief Static helper to check if input should be consumed by the UI layer.
     */
    class InputFilter
    {
    public:
        /*!
         * \brief Sets the UI system to query for input consumption.
         * \param ui Pointer to active UISystem.
         */
        static void SetUISystem(UISystem* ui)
        {
            pUI = ui;
        }

        /*!
         * \brief Checks if mouse input should be blocked from the game.
         * \return True if UI is consuming mouse input this frame.
         */
        static bool ShouldBlockMouseInput();

        /*!
         * \brief Checks if a UI element is currently hovered.
         * \return True if mouse cursor is over any UI element.
         */
        static bool IsMouseOverUI();

        /*!
         * \brief Checks if keyboard input should be blocked from the game.
         * \return True if UI text input is active.
         */
        static bool ShouldBlockKeyboardInput();

    private:
        static UISystem* pUI;
    };
}