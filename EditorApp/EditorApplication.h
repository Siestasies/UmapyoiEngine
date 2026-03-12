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
    class TilemapEditorManager;
    class PlayFabEditorManager;

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
        void PreUpdate(float dt) override;
        void PostUpdate(float dt) override;
        bool HandleInterruptions(float deltaTime) override;

    private:
        void SubscribeEvents();

        // Imgui Handling
        // previously imgui handler is inside the ImguiManager
        // it has been integrate to here for many good reasons
        // Imgui has to be render inside a loop NewFrame -> Render
        // 
        // but if it only stays in the imguimanager, 
        // other systems will have to make the imguimanager 
        // dirty by throwing all their rendering logic inside 
        // and eventually it gets BIGGGGGG and messy
        // 
        // hence its has to be moved out so that each system 
        // can render their UI in their own classes

        // IMGUI SPECIFIC METHODS
        void SetupImguiStyle();
        void ImguiStartFrame();
        void ImguiRender();

        void PlayFabConfiguration();

        EditorSystem* mEditorSystem;
        ImguiManager* mImguiManager;
        TilemapEditorManager* mTilemapEditorManager;
        PlayFabEditorManager* mPlayFabEditorManager;

        bool mWasFocused = true;
    };
}
