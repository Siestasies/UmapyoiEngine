#pragma once

namespace Uma_UI
{
    // Forward declaration
    class UISystem;

    /**
     * \class UIInputFilter
     * \brief Static helper to check if input should be consumed by UI layer
     *
     * Prevents UI clicks from triggering game actions by querying UISystem
     * for input consumption state each frame.
     */
    class InputFilter
    {
    public:
        /**
         * \brief Sets the UI system to query for input consumption
         * \param ui Pointer to active UISystem
         *
         * Must be called after UISystem is created and before game loop
         */
        static void SetUISystem(UISystem* ui)
        {
            pUI = ui;
        }

        /**
         * \brief Checks if mouse input should be blocked from game
         * \return true if UI is consuming mouse input this frame
         *
         * Returns true when:
         * - Mouse is over a UI element
         * - Mouse button was pressed on a UI element
         * - UI is actively being interacted with
         */
        static bool ShouldBlockMouseInput();

        /**
         * \brief Checks if a UI element is currently hovered
         * \return true if mouse cursor is over any UI element
         *
         * Useful for disabling camera rotation when mouse is over UI
         */
        static bool IsMouseOverUI();

        /**
         * \brief Checks if keyboard input should be blocked from game
         * \return true if UI text input is active
         *
         * Currently returns false. Will return true when InputField
         * component is added and has focus.
         */
        static bool ShouldBlockKeyboardInput();

    private:
        static UISystem* pUI;
    };
}