/*!
\file   BuildSizeAnalyzer.cpp
\par    Project: GAM250
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 4

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
       Implements the Build Size Analyzer editor panel for asset reviewing,
       selection persistence, and export functionality.


All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "BuildSizeAnalyzer.h"

#include "Core/FilePaths.h"

#include <algorithm>
#include <filesystem>
#include <fstream>

#ifdef _WIN32
#include <shlobj.h>
#endif

namespace Uma_Engine
{
    void BuildSizeAnalyzer::Init()
    {
        m_buildSelectionConfigPath = Uma_FilePath::CONFIG_ROOT + "build_selection.cfg";
    }

    void BuildSizeAnalyzer::Update(float dt)
    {
        if (m_exportResultTimer > 0.f)
            m_exportResultTimer -= dt;

        if (showWindow)
            RenderMainWindow();

        // Process export one file per frame so the UI stays responsive
        if (m_exporting)
        {
            namespace fs = std::filesystem;

            int idx = 0;
            for (auto& asset : m_buildAssets)
            {
                if (!asset.selected) continue;
                if (idx == m_exportCurrent)
                {
                    m_exportCurrentFile = asset.relativePath;
                    fs::path destPath = fs::path(m_exportDestRoot) / asset.relativePath;
                    try
                    {
                        fs::create_directories(destPath.parent_path());
                        fs::copy_file(asset.absolutePath, destPath, fs::copy_options::overwrite_existing);
                    }
                    catch (const std::exception&) { m_exportFailed++; }

                    m_exportCurrent++;
                    break;
                }
                idx++;
            }

            if (m_exportCurrent >= m_exportTotal)
            {
                m_exporting = false;
                int succeeded = m_exportTotal - m_exportFailed;
                m_exportResultMsg = "Export complete: " + std::to_string(succeeded) + " files exported";
                if (m_exportFailed > 0)
                    m_exportResultMsg += ", " + std::to_string(m_exportFailed) + " failed";
                m_exportResultTimer = 5.f;
            }
        }

        // Progress popup
        if (m_exporting)
            ImGui::OpenPopup("Exporting...");

        if (ImGui::BeginPopupModal("Exporting...", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
        {
            if (!m_exporting)
            {
                ImGui::CloseCurrentPopup();
            }
            else
            {
                float progress = m_exportTotal > 0
                    ? static_cast<float>(m_exportCurrent) / static_cast<float>(m_exportTotal)
                    : 0.f;
                ImGui::Text("Exporting assets...");
                ImGui::ProgressBar(progress, ImVec2(350, 0));
                ImGui::Text("%d / %d", m_exportCurrent, m_exportTotal);
                if (m_exportFailed > 0)
                    ImGui::TextColored(ImVec4(1.f, 0.4f, 0.4f, 1.f), "%d failed", m_exportFailed);
                ImGui::TextWrapped("Current: %s", m_exportCurrentFile.c_str());
            }
            ImGui::EndPopup();
        }
    }

    void BuildSizeAnalyzer::Shutdown()
    {
        if (m_buildAssetsScanned)
            SaveBuildSelection();
    }

    void BuildSizeAnalyzer::ScanAssets()
    {
        m_buildAssets.clear();
        std::string assetsRoot = Uma_FilePath::ASSET_ROOT;

        namespace fs = std::filesystem;
        if (!fs::exists(assetsRoot) || !fs::is_directory(assetsRoot))
            return;

        for (auto& entry : fs::recursive_directory_iterator(assetsRoot))
        {
            if (!entry.is_regular_file())
                continue;

            AssetEntry asset;
            asset.absolutePath = entry.path().string();
            asset.relativePath = fs::relative(entry.path(), assetsRoot).string();
            asset.fileSize = entry.file_size();
            asset.selected = true;
            m_buildAssets.push_back(asset);
        }

        std::sort(m_buildAssets.begin(), m_buildAssets.end(),
            [](const AssetEntry& a, const AssetEntry& b) { return a.relativePath < b.relativePath; });

        m_buildAssetsScanned = true;
        LoadBuildSelection();
    }

    void BuildSizeAnalyzer::SaveBuildSelection()
    {
        std::ofstream file(m_buildSelectionConfigPath);
        if (!file.is_open())
            return;

        for (auto& asset : m_buildAssets)
            file << (asset.selected ? "1" : "0") << "|" << asset.relativePath << "\n";
    }

    void BuildSizeAnalyzer::LoadBuildSelection()
    {
        std::ifstream file(m_buildSelectionConfigPath);
        if (!file.is_open())
            return;

        std::unordered_map<std::string, bool> selectionMap;
        std::string line;
        while (std::getline(file, line))
        {
            if (line.size() < 3)
                continue;
            bool selected = (line[0] == '1');
            std::string path = line.substr(2);
            selectionMap[path] = selected;
        }

        for (auto& asset : m_buildAssets)
        {
            auto it = selectionMap.find(asset.relativePath);
            if (it != selectionMap.end())
                asset.selected = it->second;
        }
    }

    void BuildSizeAnalyzer::RenderMainWindow()
    {
        if (!m_buildAssetsScanned)
            ScanAssets();

        ImGui::Begin("Build Size Analyzer", &showWindow);

        auto FormatSize = [](uintmax_t bytes) -> std::string
        {
            if (bytes >= 1024ULL * 1024ULL * 1024ULL)
                return std::to_string(bytes / (1024ULL * 1024ULL * 1024ULL)) + "." +
                       std::to_string((bytes % (1024ULL * 1024ULL * 1024ULL)) * 10 / (1024ULL * 1024ULL * 1024ULL)) + " GB";
            if (bytes >= 1024ULL * 1024ULL)
                return std::to_string(bytes / (1024ULL * 1024ULL)) + "." +
                       std::to_string((bytes % (1024ULL * 1024ULL)) * 10 / (1024ULL * 1024ULL)) + " MB";
            if (bytes >= 1024ULL)
                return std::to_string(bytes / 1024ULL) + "." +
                       std::to_string((bytes % 1024ULL) * 10 / 1024ULL) + " KB";
            return std::to_string(bytes) + " B";
        };

        // 0=Images, 1=Sounds, 2=Scenes, 3=Prefabs, 4=Scripts, 5=Shaders, 6=Fonts, 7=Other
        auto GetCategory = [](const std::string& path) -> int
        {
            namespace fs = std::filesystem;
            std::string ext = fs::path(path).extension().string();
            for (auto& c : ext) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

            if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga" || ext == ".gif")
                return 0;
            if (ext == ".wav" || ext == ".mp3" || ext == ".ogg" || ext == ".flac" || ext == ".bank" || ext == ".strings")
                return 1;
            if (ext == ".scn")
                return 2;
            if (ext == ".prefab")
                return 3;
            if (ext == ".lua" || ext == ".json" || ext == ".cfg" || ext == ".txt")
                return 4;
            if (ext == ".vert" || ext == ".frag" || ext == ".glsl" || ext == ".hlsl")
                return 5;
            if (ext == ".ttf" || ext == ".otf")
                return 6;
            return 7;
        };

        static const char* kCategoryNames[] = { "Images", "Sounds", "Scenes", "Prefabs", "Scripts & Data", "Shaders", "Fonts", "Other" };
        static const ImVec4 kCategoryColors[] = {
            ImVec4(0.4f, 0.8f, 0.4f, 1.0f),
            ImVec4(1.0f, 0.7f, 0.3f, 1.0f),
            ImVec4(0.4f, 0.7f, 1.0f, 1.0f),
            ImVec4(0.9f, 0.5f, 0.9f, 1.0f),
            ImVec4(1.0f, 1.0f, 0.4f, 1.0f),
            ImVec4(0.6f, 0.9f, 0.9f, 1.0f),
            ImVec4(1.0f, 0.6f, 0.6f, 1.0f),
            ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
        };
        constexpr int kNumCategories = 8;

        // Toolbar
        if (ImGui::Button("Rescan Assets"))
        {
            m_buildAssetsScanned = false;
            ScanAssets();
        }
        ImGui::SameLine();
        if (ImGui::Button("Select All"))
        {
            for (auto& asset : m_buildAssets) asset.selected = true;
            SaveBuildSelection();
        }
        ImGui::SameLine();
        if (ImGui::Button("Deselect All"))
        {
            for (auto& asset : m_buildAssets) asset.selected = false;
            SaveBuildSelection();
        }

        // Totals
        uintmax_t totalSize = 0, selectedSize = 0;
        int totalCount = static_cast<int>(m_buildAssets.size()), selectedCount = 0;
        for (auto& asset : m_buildAssets)
        {
            totalSize += asset.fileSize;
            if (asset.selected) { selectedSize += asset.fileSize; selectedCount++; }
        }

        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Selected: %d/%d assets  |  %s / %s",
            selectedCount, totalCount,
            FormatSize(selectedSize).c_str(), FormatSize(totalSize).c_str());
        ImGui::Separator();

        // Per-category stats
        uintmax_t catTotalSize[kNumCategories] = {};
        uintmax_t catSelSize[kNumCategories]   = {};
        int       catTotalCount[kNumCategories] = {};
        int       catSelCount[kNumCategories]   = {};
        for (auto& asset : m_buildAssets)
        {
            int cat = GetCategory(asset.relativePath);
            catTotalSize[cat]  += asset.fileSize;
            catTotalCount[cat] += 1;
            if (asset.selected) { catSelSize[cat] += asset.fileSize; catSelCount[cat]++; }
        }

        // Size breakdown bar
        if (totalSize > 0)
        {
            ImGui::Text("Size breakdown:");
            float barWidth = ImGui::GetContentRegionAvail().x;
            ImVec2 barPos = ImGui::GetCursorScreenPos();
            float barHeight = 16.0f;
            ImDrawList* dl = ImGui::GetWindowDrawList();
            float x = barPos.x;
            for (int c = 0; c < kNumCategories; ++c)
            {
                if (catTotalSize[c] == 0) continue;
                float segW = barWidth * (static_cast<float>(catTotalSize[c]) / static_cast<float>(totalSize));
                ImVec4 col = kCategoryColors[c];
                dl->AddRectFilled(ImVec2(x, barPos.y), ImVec2(x + segW, barPos.y + barHeight),
                    IM_COL32(static_cast<int>(col.x * 255), static_cast<int>(col.y * 255),
                             static_cast<int>(col.z * 255), 220));
                x += segW;
            }
            ImGui::Dummy(ImVec2(barWidth, barHeight));

            for (int c = 0; c < kNumCategories; ++c)
            {
                if (catTotalSize[c] == 0) continue;
                ImGui::SameLine();
                ImGui::TextColored(kCategoryColors[c], "%s", kCategoryNames[c]);
            }
            ImGui::NewLine();
        }
        ImGui::Separator();

        // Asset list grouped by category
        bool selectionChanged = false;
        float listHeight = ImGui::GetContentRegionAvail().y - 40.0f;
        ImGui::BeginChild("AssetListChild", ImVec2(0, listHeight), false);

        for (int cat = 0; cat < kNumCategories; ++cat)
        {
            if (catTotalCount[cat] == 0) continue;

            std::string header = std::string(kCategoryNames[cat]) +
                "  [" + std::to_string(catSelCount[cat]) + "/" + std::to_string(catTotalCount[cat]) +
                "  " + FormatSize(catSelSize[cat]) + " / " + FormatSize(catTotalSize[cat]) + "]";

            ImGui::PushStyleColor(ImGuiCol_Header,        ImVec4(kCategoryColors[cat].x * 0.3f, kCategoryColors[cat].y * 0.3f, kCategoryColors[cat].z * 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(kCategoryColors[cat].x * 0.45f, kCategoryColors[cat].y * 0.45f, kCategoryColors[cat].z * 0.45f, 1.0f));
            bool open = ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen);
            ImGui::PopStyleColor(2);

            if (!open) continue;

            ImGui::PushID(cat);
            if (ImGui::SmallButton("All"))
            {
                for (auto& asset : m_buildAssets)
                    if (GetCategory(asset.relativePath) == cat) asset.selected = true;
                selectionChanged = true;
            }
            ImGui::SameLine();
            if (ImGui::SmallButton("None"))
            {
                for (auto& asset : m_buildAssets)
                    if (GetCategory(asset.relativePath) == cat) asset.selected = false;
                selectionChanged = true;
            }
            ImGui::PopID();

            if (ImGui::BeginTable(kCategoryNames[cat], 3,
                ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp))
            {
                ImGui::TableSetupColumn("##chk",  ImGuiTableColumnFlags_WidthFixed, 28.0f);
                ImGui::TableSetupColumn("Path",   ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Size",   ImGuiTableColumnFlags_WidthFixed, 90.0f);
                ImGui::TableHeadersRow();

                for (size_t i = 0; i < m_buildAssets.size(); ++i)
                {
                    auto& asset = m_buildAssets[i];
                    if (GetCategory(asset.relativePath) != cat) continue;

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::PushID(static_cast<int>(i));
                    if (ImGui::Checkbox("##s", &asset.selected))
                        selectionChanged = true;
                    ImGui::PopID();

                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted(asset.relativePath.c_str());

                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%s", FormatSize(asset.fileSize).c_str());
                }
                ImGui::EndTable();
            }
            ImGui::Spacing();
        }
        ImGui::EndChild();

        if (selectionChanged)
            SaveBuildSelection();

        // Export button
        bool canExport = !m_exporting;
        if (!canExport) ImGui::BeginDisabled();
        if (ImGui::Button("Export Selected Assets", ImVec2(-1, 0)))
        {
#ifdef _WIN32
            BROWSEINFOA bi = {};
            bi.hwndOwner = GetActiveWindow();
            bi.lpszTitle = "Select Export Destination Folder";
            bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

            LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);
            if (pidl)
            {
                char folderPath[MAX_PATH] = {};
                if (SHGetPathFromIDListA(pidl, folderPath))
                {
                    m_exportDestRoot = std::string(folderPath);
                    m_exportCurrent = 0;
                    m_exportFailed = 0;
                    m_exportCurrentFile.clear();
                    m_exportTotal = 0;
                    for (auto& asset : m_buildAssets)
                        if (asset.selected) m_exportTotal++;
                    m_exporting = true;
                }
                CoTaskMemFree(pidl);
            }
#endif
        }
        if (!canExport) ImGui::EndDisabled();

        if (m_exportResultTimer > 0.f)
            ImGui::TextColored(ImVec4(0.4f, 1.f, 0.4f, 1.f), "%s", m_exportResultMsg.c_str());

        ImGui::End();

        if (!showWindow)
            SaveBuildSelection();
    }
}
