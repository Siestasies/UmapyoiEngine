#pragma once
/*!
\file   FileBrowser.hpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author     Lai Jun Siang (100%)
\par        E-mail: lai.j@digipen.edu
\par        DigiPen login: lai.j

\brief
Declares the FileBrowser class, responsible for rendering and managing the
editor-side file navigation interface.

Provides directory traversal, sorting, filtering, drag-and-drop operations,
file manipulation (copy, delete, rename), prefab editing workflow integration,
and automatic directory refresh via event callbacks. Supports real-time
interaction with the underlying filesystem using ImGui, with additional
workflow-specific features such as scene loading, prefab editing mode
switching, and directory monitoring.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <GLFW/glfw3native.h>
#include "../EditorApp/imgui/imgui.h"
#include "DropCallback.hpp"
#include "Events/IMGUIEvents.h"
#include "Events/ECSEvents.h"
#include "Events/EditorEvents.h"
#include "Systems/Graphics.hpp"

#include "Core/FilePaths.h"

namespace Uma_Engine
{
    namespace fs = std::filesystem;

    struct File {
        std::string name;
        std::string path;
        std::string stem;
        std::string ext;
        bool isFolder;
        uintmax_t size;
        fs::file_time_type last_modified;

        File(const fs::directory_entry& entry)
            : name(entry.path().filename().string())
            , path(entry.path().string())
            , stem(entry.path().stem().string())
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
        FileBrowser(const std::string& root_path = Uma_FilePath::ASSET_ROOT)
            : mCurrPath(fs::absolute(root_path))
            , eSortMode(SortMode::Name)
            , bSortAscending(true)
            , mPrefabEdit(false)
            , mPrefabSceneName("prefabEditor")
            , mPrevSceneName("")
            , mPrefabName("")
        {
            mFilter[0] = '\0';
            RefreshDirectory();
        }

        ~FileBrowser()
        {
            UnloadAllTextures();

            if (pGraphics)
            {
                if (mDefaultFileIcon != 0)
                    pGraphics->UnloadTexture(mDefaultFileIcon);
                if (mDefaultFolderIcon != 0)
                    pGraphics->UnloadTexture(mDefaultFolderIcon);
            }
        }

        void Render() {
            // Lock mutex to ensure thread-safe logging
            std::lock_guard<std::mutex> lock(mFileSysMutex);

            ImGui::Begin("File Browser");

            RenderNavigationBar();
            ImGui::Separator();

            RenderFilterBar();
            ImGui::Separator();

            // Process texture loading queue
            ProcessTextureQueue();

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

            pEventSystem->Subscribe<RefreshDirectoryRequest, FileBrowser>(
              [this](const RefreshDirectoryRequest& e) {
                    (void)e;
                    RefreshDirectory();
              });
        }

        bool isPrefabEdit()
        {
            return mPrefabEdit;
        }

        void setIsPrefabEdit(bool edit)
        {
            mPrefabEdit = edit;
        }

        std::string getPrefabName()
        {
            return mPrefabName;
        }

        std::string getPrefabSceneName()
        {
            return mPrefabSceneName;
        }

        void setPrevSceneName(std::string name)
        {
            mPrevSceneName = name;
        }

        std::string getPrevSceneName()
        {
            return mPrevSceneName;
        }

        void setGraphicsSystem(Graphics* graphics)
        {
            pGraphics = graphics;

            // Load default icons
            LoadDefaultIcons();
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
        // Prefab Edit
        bool mPrefabEdit;
        std::string mPrefabSceneName;
        std::string mPrevSceneName;
        std::string mPrefabName;

        // Texture
        Graphics* pGraphics = nullptr;
        GLuint mDefaultFileIcon = 0;
        GLuint mDefaultFolderIcon = 0;
        std::unordered_map<std::string, GLuint> mLoadedTextures;
        std::queue<std::string> mLoadQueue;
        size_t mMaxLoadsPerFrame = 2;
        float mIconSize = 64.0f;

        void LoadDefaultIcons()
        {
            if (!pGraphics) return;

            // Load default icons
            Texture fileIcon = pGraphics->LoadTextureFromFile("Assets/Icons/file_icon.png");
            mDefaultFileIcon = fileIcon.tex_id;

            Texture folderIcon = pGraphics->LoadTextureFromFile("Assets/Icons/folder_icon.png");
            mDefaultFolderIcon = folderIcon.tex_id;
        }

        void UnloadAllTextures()
        {
            if (!pGraphics) return;

            for (auto& [path, texID] : mLoadedTextures)
            {
                pGraphics->UnloadTexture(texID);
            }
            mLoadedTextures.clear();

            // Clear the load queue
            while (!mLoadQueue.empty()) mLoadQueue.pop();
        }

        bool IsImageFile(const std::string& ext)
        {
            static const std::unordered_set<std::string> imageExts =
            {
                ".png", ".jpg", ".jpeg", ".bmp"
            };

            std::string lowerExt = ext;
            std::transform(lowerExt.begin(), lowerExt.end(), lowerExt.begin(), ::tolower);
            return imageExts.count(lowerExt) > 0;
        }

        void QueueTextureLoad(const std::string& filepath)
        {
            // Skip if already loaded
            if (mLoadedTextures.count(filepath)) return;

            // Skip if already in queue
            std::queue<std::string> temp = mLoadQueue;
            while (!temp.empty())
            {
                if (temp.front() == filepath) return;
                temp.pop();
            }

            mLoadQueue.push(filepath);
        }

        void ProcessTextureQueue()
        {
            if (!pGraphics) return;

            size_t loadsThisFrame = 0;

            while (!mLoadQueue.empty() && loadsThisFrame < mMaxLoadsPerFrame)
            {
                std::string filepath = mLoadQueue.front();
                mLoadQueue.pop();

                // Skip if already loaded
                if (mLoadedTextures.count(filepath)) continue;

                // Load the texture
                Texture tex = pGraphics->LoadTextureFromFile(filepath);
                if (tex.tex_id != 0)
                {
                    mLoadedTextures[filepath] = tex.tex_id;
                    loadsThisFrame++;
                }
            }
        }

        GLuint GetFileTexture(const File& file)
        {
            // Folders use folder icon
            if (file.isFolder)
                return mDefaultFolderIcon;

            // Check if image file
            if (!IsImageFile(file.ext))
                return mDefaultFileIcon;

            // Check if texture is already loaded
            auto it = mLoadedTextures.find(file.path);
            if (it != mLoadedTextures.end())
                return it->second;

            // Not loaded yet, so queue and return default icon
            QueueTextureLoad(file.path);
            return mDefaultFileIcon;
        }

        void RefreshDirectory() {
            // Unload old directory textures
            UnloadAllTextures();

            aFiles.clear();

            try {
                for (const auto& entry : fs::directory_iterator(mCurrPath)) {
                    aFiles.emplace_back(entry);

                    // Queue image files for loading
                    if (!entry.is_directory() && IsImageFile(entry.path().extension().string()))
                    {
                        QueueTextureLoad(entry.path().string());
                    }
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

            if (!dropSource.empty() && !(mCurrPath == fs::absolute(".")) && mCurrPath.has_parent_path() && ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* pl = ImGui::AcceptDragDropPayload(dropSource.c_str())) {
                    FilePayload plData = *(FilePayload*)pl->Data;
                    try
                    {
                        fs::copy(plData.filepath, mCurrPath.parent_path(), std::filesystem::copy_options::recursive);
                        fs::remove(plData.filepath);
                    }
                    catch (const std::exception& e)
                    {
                        feedback = e.what();
                    }
                    bDropStart = false;
                    dropSource.clear();
                    RefreshDirectory();
                }
                ImGui::EndDragDropTarget();
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
            ImGui::BeginChild("FileList", ImVec2(parentSize.x, parentSize.y - ImGui::GetTextLineHeight() - 15.f), true);

            for (const auto& entry : aFiles) {
                // Apply filter
                if (mFilter[0] != '\0' && entry.name.find(mFilter) == std::string::npos) {
                    continue;
                }

                // Get texture for this file
                GLuint texID = GetFileTexture(entry);

                bool is_selected = (mSelectedPath == entry.path);

                // Push unique ID for this item
                ImGui::PushID(entry.path.c_str());

                // Render icon + text in horizontal layout
                ImGui::BeginGroup();

                // Draw icon
                if (texID != 0)
                {
                    ImGui::Image((void*)(intptr_t)texID, ImVec2(mIconSize, mIconSize));
                }
                else
                {
                    // Fallback: just draw a colored rect
                    ImGui::Dummy(ImVec2(mIconSize, mIconSize));
                }

                ImGui::SameLine();

                // Draw text with some padding
                ImGui::BeginGroup();
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.0f); // Vertical align
                ImGui::Text("%s", entry.name.c_str());

                if (!entry.isFolder)
                {
                    ImGui::TextDisabled("Size: %llu bytes", entry.size);
                }
                ImGui::EndGroup();

                ImGui::EndGroup();

                // Invisible button over the entire group for selection
                ImVec2 itemMin = ImGui::GetItemRectMin();
                ImVec2 itemMax = ImGui::GetItemRectMax();
                ImGui::SetCursorScreenPos(itemMin);
                ImGui::InvisibleButton("##fileitem", ImVec2(itemMax.x - itemMin.x, itemMax.y - itemMin.y));

                // Selection handling
                if (ImGui::IsItemClicked())
                {
                    mSelectedPath = entry.path;
                }

                // Double-click detection
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
                {
                    mSelectedPath = entry.path;
                    if (FileDoubleClickHandler(entry))
                    {
                        ImGui::PopID();
                        break;
                    }
                }

                // Highlight selection
                if (is_selected)
                {
                    ImDrawList* draw_list = ImGui::GetWindowDrawList();
                    ImVec2 p_min = itemMin;
                    ImVec2 p_max = itemMax;
                    draw_list->AddRectFilled(p_min, p_max, IM_COL32(64, 128, 255, 80)); // Blue highlight
                    draw_list->AddRect(p_min, p_max, IM_COL32(64, 128, 255, 255), 0.0f, 0, 2.0f); // Blue border

                    // Keyboard shortcuts (only for selected item)
                    if (ImGui::IsKeyPressed(ImGuiKey_Delete))
                    {
                        try
                        {
                            fs::remove(mSelectedPath);
                            RefreshDirectory();
                            mSelectedPath.clear();
                        }
                        catch (const std::exception& e)
                        {
                            feedback = e.what();
                        }
                        ImGui::PopID();
                        break;
                    }
                    else if (ImGui::IsKeyDown(ImGuiKey_ModCtrl) && ImGui::IsKeyPressed(ImGuiKey_C))
                    {
                        copySource = entry.path;
                    }
                }

                // Context menu
                if (ImGui::BeginPopupContextItem("##contextmenu"))
                {
                    if (ImGui::MenuItem("Copy"))
                    {
                        copySource = entry.path;
                    }

                    if (ImGui::MenuItem("Delete"))
                    {
                        try
                        {
                            fs::remove(mSelectedPath);
                            RefreshDirectory();
                            mSelectedPath.clear();
                        }
                        catch (const std::exception& e)
                        {
                            feedback = e.what();
                        }
                    }

                    // Rename functionality
                    static char renameText[256];
                    renameText[255] = '\0';

                    if (ImGui::InputText("##name", renameText, 256)) {}
                    ImGui::SameLine();
                    float inputHeight = ImGui::GetFrameHeight();
                    float textHeight = ImGui::GetTextLineHeight();
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (inputHeight - textHeight) * 0.5f);

                    if (ImGui::MenuItem("Rename"))
                    {
                        std::string newName(renameText);
                        if (!newName.empty())
                        {
                            newName = fs::path(entry.path).parent_path().string() + "\\" + newName + entry.ext;
                            if (fs::exists(newName))
                                feedback = "rename: file already exists";
                            else
                            {
                                try
                                {
                                    fs::rename(entry.path, newName);
                                    RefreshDirectory();
                                    mSelectedPath.clear();
                                }
                                catch (const std::exception& e)
                                {
                                    feedback = e.what();
                                }
                            }
                        }
                    }

                    // Prefab-specific menu items
                    if (entry.ext == ".prefab" && !mPrefabEdit)
                    {
                        if (ImGui::MenuItem("Open in Inspector"))
                        {
                            pEventSystem->Emit<StopSceneRequest>();
                            pEventSystem->Emit<SaveCurrSceneRequest>();
                            pEventSystem->Emit<PrefabSceneRequestEvent>(mPrefabSceneName);
                            pEventSystem->Emit<ClearSceneRequestEvent>();
                            pEventSystem->Emit<LoadPrefabRequestEvent>(entry.name, false);
                            mPrefabName = entry.stem;
                            mPrefabEdit = true;
                        }

                        if (ImGui::MenuItem("Add to Scene"))
                        {
                            pEventSystem->Emit<LoadPrefabRequestEvent>(entry.name);
                        }
                    }

                    ImGui::EndPopup();
                }

                // Drag and drop source
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
                {
                    // Create payload
                    FilePayload payload;
                    strncpy_s(payload.filepath, entry.path.c_str(), sizeof(payload.filepath) - 1);
                    payload.filepath[sizeof(payload.filepath) - 1] = '\0';
                    payload.isFolder = entry.isFolder;

                    ImGui::SetDragDropPayload(entry.name.c_str(), &payload, sizeof(FilePayload));

                    // Preview
                    ImGui::Text("%s", entry.name.c_str());
                    ImGui::EndDragDropSource();

                    bDropStart = true;
                    dropSource = entry.name.c_str();
                }

                // Drag and drop target (only for folders)
                if (entry.isFolder && ImGui::BeginDragDropTarget())
                {
                    if (!dropSource.empty())
                    {
                        if (const ImGuiPayload* pl = ImGui::AcceptDragDropPayload(dropSource.c_str()))
                        {
                            FilePayload* plData = (FilePayload*)pl->Data;
                            try
                            {
                                fs::copy(plData->filepath, entry.path, std::filesystem::copy_options::recursive);
                                fs::remove(plData->filepath);
                                RefreshDirectory();
                            }
                            catch (const std::exception& e)
                            {
                                feedback = e.what();
                            }
                            bDropStart = false;
                            dropSource.clear();
                            ImGui::EndDragDropTarget();
                            ImGui::PopID();
                            break;
                        }
                    }
                    ImGui::EndDragDropTarget();
                }

                // Tooltip with details
                if (ImGui::IsItemHovered())
                {
                    ImGui::BeginTooltip();
                    ImGui::Text("Path: %s", entry.path.c_str());

                    if (!entry.isFolder)
                    {
                        ImGui::Text("Size: %llu bytes", entry.size);

                        // Show loading status for images
                        if (IsImageFile(entry.ext))
                        {
                            if (mLoadedTextures.count(entry.path) == 0)
                            {
                                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Loading preview...");
                            }
                        }
                    }

                    ImGui::EndTooltip();
                }

                ImGui::PopID();
            }

            ImGui::EndChild();

            // Clear selection when clicking empty space
            if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered())
            {
                mSelectedPath.clear();
            }
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

        void OpenScriptInExternalEditor(const std::string& filepath)
        {
#ifdef _WIN32
            // Window - use ShellExecuteA instead of system() - much faster and non-blocking
            ShellExecuteA(NULL, "open", filepath.c_str(), NULL, NULL, SW_SHOWNORMAL);
#elif __APPLE__
            // macOS
            std::string command = "open \"" + filepath + "\"";
            system(command.c_str());
#elif __linux__
            // Linux
            std::string command = "xdg-open \"" + filepath + "\"";
            system(command.c_str());
#endif
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
                //pEventSystem->Emit<LoadPrefabRequestEvent>(file.name);
                pEventSystem->Emit<StopSceneRequest>();
                pEventSystem->Emit<SaveCurrSceneRequest>();
                pEventSystem->Emit<PrefabSceneRequestEvent>(mPrefabSceneName);
                pEventSystem->Emit<ClearSceneRequestEvent>();
                pEventSystem->Emit<LoadPrefabRequestEvent>(file.name, false);
                mPrefabName = file.stem;
                mPrefabEdit = true;
                return true;
            }
            else if (ext == ".lua")
            {
                OpenScriptInExternalEditor(file.path);
                feedback = "opened script " + file.path;
            }
            else
            {
                feedback = "Unknown file type";
            }
            return false;
        }

    };
} // namespace Uma_Engine