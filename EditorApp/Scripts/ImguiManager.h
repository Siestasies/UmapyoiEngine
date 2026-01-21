/*!
\file   ImguiManager.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Shahir Rasid (100%)
\par    E-mail: b.muhammadshahir@digipen.edu
\par    DigiPen login: b.muhammadshahir

\brief
This file implements the definition for an IMGUI Manager as a system and
implements functions to create and show IMGUI windows including
Hierarchy panel for ECS entity management.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/
#pragma once
#include "Core/SystemType.h"
#include "Events/IMGUIEvents.h"
#include "Events/DebugEvents.h"
#include "Events/ECSEvents.h"

// external window script
#include "FileSystem/FileSystem.hpp"
#include "FileSystem/ResourcesWindow.hpp"
// Forward declaration instead of include to avoid circular dependency
// #include "SceneManager.h"

#include "ECS/Core/Types.hpp"
#include "ECS/Core/Coordinator.hpp"
#include "ECS/Components/Transform.h"
#include "ECS/Components/Player.h"
#include "ECS/Components/Enemy.h"
#include "ECS/Components/Camera.h"
#include "ECS/Components/Sprite.h"
#include "ECS/Components/AudioComponent.h"
#include "ECS/Components/AudioListener.h"
#include "ECS/Components/PathFinding.h"
#include "ECS/Components/Prefab.h"
#include "ECS/Components/Projectile.h"

#include "Systems/ResourcesManager.hpp"

// Commands related
#include "../Editor/Core/CommandHistory.h"
#include "../Editor/Core/EntitySnapshot.h"

#include "Core/FilePaths.h"
#include <iostream>
#include <random>

struct GLFWwindow;

namespace Uma_Engine
{
    // Forward declaration to avoid circular dependency
    class SceneManager;

    enum class PlayState
    {
        Stopped,
        Playing,
        Paused
    };

    class ImguiManager : public ISystem, public IWindowSystem
    {
    public:
        ImguiManager();

        // isystem stuff
        void Init() override;
        void Update(float deltaTime) override;
        void Shutdown() override;
        void SetWindow(GLFWwindow* window) override;

        // ImGui-specific methods
        void StartFrame();
        void Render();
        void CreateDebugWindows(float fps, float deltaTime);
        bool IsInitialized() const { return m_initialized; }

        // for window controls
        bool IsPlaying() const { return m_playState == PlayState::Playing; }
        bool IsPaused() const { return m_playState == PlayState::Paused; }
        bool IsStopped() const { return m_playState == PlayState::Stopped; }
        void ShowEngineDebug(bool show) { m_showEngineDebug = show; }
        void ShowEventDebug(bool show) { m_showEventDebug = show; }
        void ShowPerformanceWindow(bool show) { m_showPerformanceWindow = show; }
        void ShowSystemsWindow(bool show) { m_showSystemsWindow = show; }

        EventSystem* GetESHandler() { if (!pEventSystem) return pEventSystem; }
        SystemManager* GetSMHandler() { if (!pSystemManager) return pSystemManager; }


        Vec2 GetSceneViewMousePos() const { return m_sceneViewMousePos; }
        bool IsMouseInSceneView() const { return m_isMouseInSceneView; }

        // undo redo snapshot
        void BeginComponentEdit(Uma_ECS::Entity entity, Uma_ECS::Coordinator& coordinator);
        void EndComponentEdit(Uma_ECS::Entity entity, Uma_ECS::Coordinator& coordinator, const std::string& componentName, bool forceEdit = false);
        bool& HasUnsavedChanges();
        Uma_Editor::CommandHistory* GetCommandHistory() { return &commandHistory; }

    private:
        // bigger space stuff
        bool WindowsInit(std::string filename = Uma_FilePath::CONFIG_ROOT + "imgui.ini");
        void CreateDockspace();
        void InitDockspace(ImGuiID dockspace_id, ImGuiViewport* viewport);

        // Window creation functions
        void SceneManagerWindow();
        void CreateHierarchyWindow();
        void CreatePrefabHierarchyWindow();
        void CreateInspectorWindow();
        void CreateEditorControlBar();
        void CreatePrefabControlBar();
        void CreateSystemsWindow();
        void CreatePerformanceWindow();
        void CreateEngineDebugWindow(float fps, float deltaTime);
        void CreateConsoleWindow();
        void CreateEditorCameraWindow();
        void CreateSceneViewWindow();
        void AddConsoleLog(const std::string& message);

        // external helper function
        void OpenScriptInVSCode(const std::string& filepath);

        // Hierarchy helper functions
        void RenderEntityNode(Uma_ECS::Entity entity, Uma_ECS::Coordinator& coordinator,
            Uma_ECS::ComponentArray<Uma_ECS::Transform>& transformArray);
        void RenderPrefabNode(Uma_ECS::Entity entity, Uma_ECS::Coordinator& coordinator,
            Uma_ECS::ComponentArray<Uma_ECS::Transform>& transformArray);
        void RenderHierarchyDropZone(int insertIndex, Uma_ECS::Coordinator& coordinator,
            std::optional<Uma_ECS::Entity> parentEntity = std::nullopt);
        std::string GetEntityDisplayName(Uma_ECS::Entity entity, Uma_ECS::Coordinator& coordinator);
        bool IsChildOf(Uma_ECS::Entity potentialChild, Uma_ECS::Entity potentialParent,
            Uma_ECS::ComponentArray<Uma_ECS::Transform>& transformArray);

        // Inspector helper functions
        bool DisplayComponent(Uma_ECS::Coordinator&, Uma_ECS::ComponentType, Uma_ECS::Entity&);

        // undo redo snapshot
        Uma_Editor::EntitySnapshot CaptureEntitySnapshot(Uma_ECS::Entity entity, Uma_ECS::Coordinator& coord);
        void HandleUndoRedoInput();

        bool m_initialized;
        bool ds_initialized;
        GLFWwindow* m_window;
        std::vector<std::string> logsVec;

        // handlers
        EventSystem* pEventSystem = nullptr;
        ResourcesManager* pResourcesManager = nullptr;
        
        std::vector<std::string> sceneNames;
        std::vector<std::string> scenePaths;
        int activeSceneIndex;
        std::string mScriptName;

        // holded in external scripts
        FileBrowser fileBrowser;
        ResourcesWindow resourcesWindow;

        // hierachy 
        bool m_HierarchyScrollToBottom = false;
        int m_HierarchyScrollToBottomFrames = 0;

        // Selected entity tracking for Inspector
        Uma_ECS::Entity m_selectedEntity;
        Uma_ECS::Entity m_prefabEntity;

        // show or not
        bool m_hideAll;
        bool m_showEngineDebug;
        bool m_showEventDebug;
        bool m_showPerformanceWindow;
        bool m_showSystemsWindow;
        bool m_showEditorControlBar;
        bool m_showEditorCameraWindow;
        PlayState m_playState = PlayState::Stopped;

        // values that need to keep track
        int mEntityCount;
        int windowWidth, windowHeight;

        // performance window vars
        float m_fpsHistory[120];
        float m_dtHistory[120];
        int m_historyOffset;

        // undo redo feature
        Uma_Editor::CommandHistory commandHistory;

        // Edit session tracking
        Uma_ECS::Entity m_editingEntity;
        Uma_Editor::EntitySnapshot m_snapshotBeforeEdit;
        bool m_hasUnsavedEdit = false;

        // mouse over checks
        bool prevMouseOverUI = false;
        bool mouseOverUI = false;

        bool m_sceneViewHovered = false;
        Vec2 m_sceneViewMousePos = Vec2(-1, -1);
        bool m_isMouseInSceneView = false;

        std::string m_popupErrorMessage = "";
    };
}