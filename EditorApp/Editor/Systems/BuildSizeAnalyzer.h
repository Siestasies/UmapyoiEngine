/*!
\file   BuildSizeAnalyzer.h
\par    Project: GAM250
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 4

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
      Editor-only ImGui window for reviewing project assets, selecting assets
      for export, and monitoring total export size.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "../Core/SystemType.h"

#include <imgui.h>
#include <string>
#include <vector>
#include <unordered_map>

namespace Uma_Engine
{
    class BuildSizeAnalyzer : public ISystem
    {
    public:
        // Sets up the config path used to persist asset selection state
        void Init() override;

        // Called every frame — processes one export file per frame and renders the window
        void Update(float dt) override;

        // Saves the current selection on shutdown
        void Shutdown() override;

        // Toggles the window open/closed
        void ToggleWindow() { showWindow = !showWindow; }

        // Returns whether the window is currently visible
        bool IsWindowVisible() const { return showWindow; }

    private:
        // Draws the full Build Size Analyzer ImGui window
        void RenderMainWindow();

        // Scans the Assets/ directory and populates the asset list
        void ScanAssets();

        // Writes the current selection state to build_selection.cfg
        void SaveBuildSelection();

        // Reads and applies the saved selection state from build_selection.cfg
        void LoadBuildSelection();

        bool showWindow = false;

        struct AssetEntry
        {
            std::string relativePath;
            std::string absolutePath;
            uintmax_t fileSize = 0;
            bool selected = false;
        };
        std::vector<AssetEntry> m_buildAssets;
        bool m_buildAssetsScanned = false;
        std::string m_buildSelectionConfigPath;

        // Export progress
        bool m_exporting = false;
        int  m_exportTotal = 0;
        int  m_exportCurrent = 0;
        int  m_exportFailed = 0;
        std::string m_exportCurrentFile;
        std::string m_exportDestRoot;
        std::string m_exportResultMsg;
        float m_exportResultTimer = 0.f;
    };
}
