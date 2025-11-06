#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <GLFW/glfw3native.h>
#include "imgui.h"
#include "DropCallback.hpp"
#include "Events/IMGUIEvents.h"
#include "Events/ECSEvents.h"

#include "Core/FilePaths.h"

namespace Uma_Engine
{
    namespace fs = std::filesystem;

    struct File {
        std::string name;
        std::string path;
        std::string ext;
        bool isFolder;
        uintmax_t size;
        fs::file_time_type last_modified;

        File(const fs::directory_entry& entry)
            : name(entry.path().filename().string())
            , path(entry.path().string())
            , ext(entry.path().extension().string())
            , isFolder(entry.is_directory())
            , size(entry.is_directory() ? 0 : entry.file_size())
            , last_modified(entry.last_write_time())
        {
        }
    };

    struct FilePayload {
        char filepath[256];
        bool isFolder;
    };

    enum class SortMode {
        Name,
        Size,
        Type,
        Modified
    };

    class FileBrowser
    {
    public:
        FileBrowser(const std::string& root_path = ".")
            : mCurrPath(fs::absolute(root_path))
            , eSortMode(SortMode::Name)
            , bSortAscending(true)
        {
            mFilter[0] = '\0';
            RefreshDirectory();
        }

        void Render() {
            // Lock mutex to ensure thread-safe logging
            std::lock_guard<std::mutex> lock(mFileSysMutex);

            ImGui::Begin("File Browser");

            RenderNavigationBar();
            ImGui::Separator();

            RenderFilterBar();
            ImGui::Separator();

            RenderFileList();

            if (!FileDropHandler::aDroppedFiles.empty() && ImGui::IsWindowHovered()) {
                for (const auto& droppedfilepath : FileDropHandler::aDroppedFiles) {
                    try
                    {
                        fs::copy(droppedfilepath, mCurrPath, std::filesystem::copy_options::recursive);
                        RefreshDirectory();
                    }
                    catch (const std::exception& e)
                    {
                        feedback = e.what();
                    }
                }
                FileDropHandler::aDroppedFiles.clear();
            }
            //Paste
            if (ImGui::IsKeyDown(ImGuiKey_ModCtrl) && ImGui::IsKeyPressed(ImGuiKey_V) && !copySource.empty())
            {
                try
                {
                    fs::copy(copySource, mCurrPath, std::filesystem::copy_options::recursive);
                    RefreshDirectory();
                }
                catch (const std::exception& e)
                {
                    feedback = e.what();
                }
            }

            RenderFeedback();

            ImGui::End();
        }

        static bool fileExists(std::string filepath)
        {
            if (!fs::exists(filepath))
                return false;

            std::filesystem::path fileParent(filepath);
            fileParent = fileParent.parent_path();
            std::string filename = filepath;
            filename = filename.erase(0, 15);
            for (const auto& entry : std::filesystem::directory_iterator(fileParent)) {
                if (entry.path().filename() == filename) {
                    return true;
                }
            }
            return false;
        }

        void setEventSystem(EventSystem* es)
        {
            pEventSystem = es;

            pEventSystem->Subscribe<RefreshDirectoryRequest>(
              [this](const RefreshDirectoryRequest& e) {
                    RefreshDirectory();
              });
        }

    private:
        // Files
        fs::path mCurrPath;
        std::string mSelectedPath;
        std::vector<File> aFiles;
        char mFilter[256];
        // Sort
        SortMode eSortMode;
        bool bSortAscending;
        // Mouse
        bool bDoubleClick = false;
        // Drag Drop
        bool bDropStart = false;
        std::string dropSource;
        // Copy
        std::string copySource;
        // Log
        std::string feedback = ":P";
        // Mutex
        std::mutex mFileSysMutex;
        // Events
        EventSystem* pEventSystem = nullptr;

        void RefreshDirectory() {
            aFiles.clear();

            try {
                for (const auto& entry : fs::directory_iterator(mCurrPath)) {
                    aFiles.emplace_back(entry);
                }
                SortFiles();
            }
            catch (const fs::filesystem_error& e) {
                feedback = e.what();
            }
        }

        void SortFiles() {
            std::sort(aFiles.begin(), aFiles.end(), [this](const File& a, const File& b) {
                // Directories always first
                if (a.isFolder != b.isFolder)
                    return a.isFolder;

                bool result = false;
                switch (eSortMode) {
                case SortMode::Name:
                    result = a.name < b.name;
                    break;
                case SortMode::Size:
                    result = a.size < b.size;
                    break;
                case SortMode::Type: {
                    auto ext_a = fs::path(a.name).extension().string();
                    auto ext_b = fs::path(b.name).extension().string();
                    result = ext_a < ext_b;
                    break;
                }
                case SortMode::Modified:
                    result = a.last_modified < b.last_modified;
                    break;
                }

                return bSortAscending ? result : !result;
                });
        }

        void RenderNavigationBar() {
            // Up directory button
            if (ImGui::Button("^ Up") && !(mCurrPath == fs::absolute(".")) && mCurrPath.has_parent_path()) {
                mCurrPath = mCurrPath.parent_path();
                RefreshDirectory();
            }

            ImGui::SameLine();
            ImGui::Text("Path:");

            ImGui::SameLine();
            ImGui::TextWrapped("%s", fs::relative(mCurrPath, fs::absolute(".")).string().c_str());

            // Quick navigation buttons
            ImGui::SameLine();
            if (ImGui::Button("Refresh")) {
                RefreshDirectory();
            }
        }

        void RenderFilterBar() {
            ImGui::SetNextItemWidth(200);
            ImGui::InputText("Filter", mFilter, IM_ARRAYSIZE(mFilter));

            ImGui::SameLine();

            // Sort mode selector
            const char* sort_modes[] = { "Name", "Size", "Type", "Modified" };
            int current_mode = static_cast<int>(eSortMode);
            ImGui::SetNextItemWidth(100);
            if (ImGui::Combo("Sort", &current_mode, sort_modes, IM_ARRAYSIZE(sort_modes))) {
                eSortMode = static_cast<SortMode>(current_mode);
                SortFiles();
            }

            ImGui::SameLine();
            if (ImGui::Button(bSortAscending ? "Asc" : "Desc")) {
                bSortAscending = !bSortAscending;
                SortFiles();
            }
        }

        void RenderFileList() {
            bDoubleClick = false;

            ImVec2 parentSize = ImGui::GetContentRegionAvail();
            ImGui::CalcTextSize(feedback.c_str());
            ImGui::BeginChild("FileList", ImVec2(parentSize.x, parentSize.y - ImGui::GetTextLineHeight() - 15.f), true);
            for (const auto& entry : aFiles) {
                // Apply filter
                if (mFilter[0] != '\0' &&
                    entry.name.find(mFilter) == std::string::npos) {
                    continue;
                }

                std::string fileName = entry.isFolder ? "[FOLDER] " : "[FILE] ";
                fileName += entry.name;

                bool is_selected = (mSelectedPath == entry.path);

                if (ImGui::Selectable(fileName.c_str(), is_selected, ImGuiSelectableFlags_AllowDoubleClick)) {
                    mSelectedPath = entry.path;

                    // Double-click
                    if (ImGui::IsMouseDoubleClicked(0)) {
                        if (FileDoubleClickHandler(entry))
                            break;
                    }
                }

                if (is_selected)
                {
                    if (ImGui::IsKeyPressed(ImGuiKey_Delete)) // Delete
                    {
                        fs::remove(mSelectedPath);
                        RefreshDirectory();
                        mSelectedPath.clear();
                        break;
                    }
                    else if (ImGui::IsKeyDown(ImGuiKey_ModCtrl) && ImGui::IsKeyPressed(ImGuiKey_C)) // Copy
                    {
                        copySource = entry.path;
                    }
                }

                if (ImGui::BeginPopupContextItem())
                {
                    if (ImGui::MenuItem("Copy"))
                    {
                        copySource = entry.path;
                    }
                    if (ImGui::MenuItem("Delete"))
                    {
                        fs::remove(mSelectedPath);
                        RefreshDirectory();
                        mSelectedPath.clear();
                    }
                    ImGui::EndPopup();
                }

                // Drag and drop source
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                    // Store the full path in the payload
                    ImGui::SetDragDropPayload(entry.name.c_str(), entry.path.c_str(), entry.path.size() + 1);
                    ImGui::Text("%s", entry.name.c_str());
                    ImGui::EndDragDropSource();

                    bDropStart = true;
                    dropSource = entry.name.c_str();
                }

                if (entry.isFolder && ImGui::BeginDragDropTarget() && !dropSource.empty()) {
                    if (const ImGuiPayload* pl = ImGui::AcceptDragDropPayload(dropSource.c_str())) {
                        FilePayload plData = *(FilePayload*)pl->Data;
                        bool success = false;
                        try
                        {
                            fs::copy(plData.filepath, entry.path, std::filesystem::copy_options::recursive);
                            fs::remove(plData.filepath);
                        }
                        catch (const std::exception& e)
                        {
                            feedback = e.what();
                        }
                        bDropStart = false;
                        dropSource.clear();
                        RefreshDirectory();
                        break;
                    }
                    ImGui::EndDragDropTarget();
                }

                // Tooltip with details
                if (ImGui::IsItemHovered()) {
                    ImGui::BeginTooltip();
                    ImGui::Text("Path: %s", entry.path.c_str());
                    if (!entry.isFolder) {
                        ImGui::Text("Size: %llu bytes", entry.size);
                    }
                    ImGui::EndTooltip();

                }

            }

            ImGui::EndChild();
        }

        void RenderFeedback()
        {
            if (feedback.empty())
                return;
            ImGuiWindowFlags flags = 0;
            flags |= ImGuiWindowFlags_NoScrollbar;          // No scrollbars at all
            flags |= ImGuiWindowFlags_NoScrollWithMouse;    // Can't scroll with mouse wheel
            ImVec2 parentSize = ImGui::GetContentRegionAvail();
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.f, 5.f));
            ImGui::BeginChild("Feedback", ImVec2(parentSize.x, 0), true, flags);
            ImGui::Text("%s", feedback.c_str());
            ImGui::EndChild();
            ImGui::PopStyleVar();
        }

        bool FileDoubleClickHandler(const File& file)
        {
            if (file.isFolder) {
                mCurrPath = file.path;
                RefreshDirectory();
                mSelectedPath.clear();
                return true;
            }
            std::string ext = file.ext;
            if (ext == ".scn")
            {
                if (pEventSystem != nullptr)
                {
                    /*std::string newPath = Uma_FilePath::SCENES_DIR;
                    newPath += file.name;*/
                    pEventSystem->Emit<LoadSceneRequestEvent>(file.name);
                    return true;
                }
            }
            else if (ext == ".prefab")
            {
                pEventSystem->Emit<LoadPrefabRequestEvent>(file.name);
                return true;
            }
            else
            {
                feedback = "Unknown file type";
            }
            return false;
        }

    };

} // namespace Uma_Engine