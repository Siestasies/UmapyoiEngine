/*!
\file   EditorApplication.h
\brief  Editor application that extends the base Application with editor-specific systems

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "Core/Application.h"

namespace Uma_Engine
{
    class EditorSystem;
    class ImguiManager;

    /**
     * \brief Editor application with full editor functionality
     *
     * This application includes:
     * - EditorSystem (entity picking, gizmo manipulation, undo/redo)
     * - ImguiManager (hierarchy, inspector, scene view UI)
     * - EditorScript (editor-specific scene behaviors)
     */
    class EditorApplication : public Application
    {
    public:
        EditorApplication();
        ~EditorApplication() override = default;

    protected:
        void RegisterSystems() override;
        void PostInit() override;
        void PreInit() override;

    private:
        void SubscribeEvents();

        EditorSystem* mEditorSystem;
        ImguiManager* mImguiManager;
    };
}
