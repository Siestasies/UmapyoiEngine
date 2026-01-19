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

#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#endif

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
        std::unordered_map<std::string, Texture> mLoadedTextures;
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

            for (auto& [path, tex] : mLoadedTextures)
            {
                pGraphics->UnloadTexture(tex.tex_id);
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

        std::string FormatFileSize(uintmax_t bytes)
        {
            const char* units[] = { "B", "KB", "MB", "GB", "TB" };
            int unitIndex = 0;
            double size = static_cast<double>(bytes);

            while (size >= 1024.0 && unitIndex < 4)
            {
                size /= 1024.0;
                unitIndex++;
            }

            char buffer[64];
            if (unitIndex == 0)
            {
                // Bytes=
                snprintf(buffer, sizeof(buffer), "%d %s", static_cast<int>(size), units[unitIndex]);
            }
            else
            {
                // KB, MB, GB, TB
                snprintf(buffer, sizeof(buffer), "%.2f %s", size, units[unitIndex]);
            }

            return std::string(buffer);
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
                    mLoadedTextures[filepath] = tex;
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
                return it->second.tex_id;

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

            ImGui::SameLine();
            if (ImGui::Button("Upload File")) {
                std::string path = OpenFileDialog();
                if (!path.empty()) {
                    try {
                        fs::path source = path;
                        fs::path target = mCurrPath / source.filename();

                        if (fs::exists(target)) {
                            feedback = "Error: File already exists in this folder.";
                        }
                        else {
                            fs::copy(source, target);
                            RefreshDirectory();
                            feedback = "Uploaded: " + source.filename().string();
                        }
                    }
                    catch (const std::exception& e) {
                        feedback = std::string("Upload Error: ") + e.what();
                    }
                }
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

            // Grid layout settings
            const float itemWidth = mIconSize + 20.0f;
            const float itemHeight = mIconSize + 50.0f;
            const float itemSpacing = 15.0f;

            // Calculate items per row
            float availableWidth = ImGui::GetContentRegionAvail().x;
            int itemsPerRow = max(1, static_cast<int>((availableWidth + itemSpacing) / (itemWidth + itemSpacing)));

            int currentColumn = 0;
            ImVec2 startPos = ImGui::GetCursorPos();

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

                // Calculate grid position
                float xPos = startPos.x + (currentColumn * (itemWidth + itemSpacing));

                // Set cursor to grid position
                if (currentColumn > 0) {
                    ImGui::SameLine(xPos);
                }

                // Begin item group
                ImGui::BeginGroup();

                // Icon
                float iconPosX = (itemWidth - mIconSize) * 0.5f;
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + iconPosX);

                if (texID != 0)
                {
                    ImGui::Image((void*)(intptr_t)texID, ImVec2(mIconSize, mIconSize));
                }
                else
                {
                    ImGui::Dummy(ImVec2(mIconSize, mIconSize));
                }

                // Small spacing between icon and text
                ImGui::Spacing();

                // File name
                std::string displayName = entry.name;
                const size_t maxNameLength = 18;
                if (displayName.length() > maxNameLength)
                {
                    displayName = displayName.substr(0, maxNameLength - 3) + "...";
                }

                // Push text wrapping
                ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + itemWidth);

                float textWidth = ImGui::CalcTextSize(displayName.c_str(), nullptr, false, itemWidth).x;
                float textPosX = (itemWidth - textWidth) * 0.5f;
                if (textPosX > 0) {
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + textPosX);
                }

                ImGui::Text("%s", displayName.c_str());
                ImGui::PopTextWrapPos();

                // Dummy to ensure consistent height
                ImGui::Dummy(ImVec2(itemWidth, 0));

                ImGui::EndGroup();

                // Store item rect for interaction
                ImVec2 itemMin = ImGui::GetItemRectMin();
                ImVec2 itemMax = ImGui::GetItemRectMax();

                // Make item selectable (invisible button over the group)
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

                    // Add padding to selection rect
                    p_min.x -= 5.0f;
                    p_min.y -= 5.0f;
                    p_max.x += 5.0f;
                    p_max.y += 5.0f;

                    draw_list->AddRectFilled(p_min, p_max, IM_COL32(64, 128, 255, 80));
                    draw_list->AddRect(p_min, p_max, IM_COL32(64, 128, 255, 255), 4.0f, 0, 2.0f);

                    // Keyboard shortcuts
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
                    FilePayload payload;
                    strncpy_s(payload.filepath, entry.path.c_str(), sizeof(payload.filepath) - 1);
                    payload.filepath[sizeof(payload.filepath) - 1] = '\0';
                    payload.isFolder = entry.isFolder;

                    ImGui::SetDragDropPayload(entry.name.c_str(), &payload, sizeof(FilePayload));
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

                // Hover preview
                if (ImGui::IsItemHovered())
                {
                    ImGui::BeginTooltip();

                    // Show large preview for images with proper aspect ratio
                    if (IsImageFile(entry.ext) && mLoadedTextures.count(entry.path) > 0)
                    {
                        const Texture& tex = mLoadedTextures[entry.path];
                        GLuint previewTexID = tex.tex_id;

                        // Maximum preview size (constrain to fit in tooltip)
                        const float maxPreviewSize = 256.0f;

                        // Calculate aspect-ratio-preserving size
                        float texWidth = tex.tex_size.x;
                        float texHeight = tex.tex_size.y;

                        ImVec2 previewSize;
                        if (texWidth > texHeight)
                        {
                            // Width is limiting factor
                            previewSize.x = maxPreviewSize;
                            previewSize.y = (texHeight / texWidth) * maxPreviewSize;
                        }
                        else
                        {
                            // Height is limiting factor
                            previewSize.y = maxPreviewSize;
                            previewSize.x = (texWidth / texHeight) * maxPreviewSize;
                        }

                        ImGui::Image((void*)(intptr_t)previewTexID, previewSize);
                        ImGui::Separator();
                    }

                    // File details
                    ImGui::Text("Name: %s", entry.name.c_str());
                    ImGui::Text("Path: %s", entry.path.c_str());

                    if (!entry.isFolder)
                    {
                        ImGui::Text("Size: %s", FormatFileSize(entry.size).c_str());
                        ImGui::Text("Type: %s", entry.ext.c_str());

                        // Show dimensions for images
                        if (IsImageFile(entry.ext) && mLoadedTextures.count(entry.path) > 0)
                        {
                            const Texture& tex = mLoadedTextures[entry.path];
                            ImGui::Text("Dimensions: %.0f x %.0f", tex.tex_size.x, tex.tex_size.y);
                        }
                        else if (IsImageFile(entry.ext))
                        {
                            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Loading preview...");
                        }
                    }
                    else
                    {
                        ImGui::Text("Type: Folder");
                    }

                    ImGui::EndTooltip();
                }

                ImGui::PopID();

                // Update column counter and handle wrapping
                currentColumn++;
                if (currentColumn >= itemsPerRow)
                {
                    currentColumn = 0;
                    // Add vertical spacing between rows
                    ImGui::Dummy(ImVec2(0, itemSpacing));
                }
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

        std::string OpenFileDialog()
        {
#ifdef _WIN32
            OPENFILENAMEA ofn;
            CHAR szFile[260] = { 0 };
            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = GetActiveWindow();
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrFilter = "All Files\0*.*\0";
            ofn.nFilterIndex = 1;
            ofn.lpstrFileTitle = NULL;
            ofn.nMaxFileTitle = 0;
            ofn.lpstrInitialDir = NULL;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

            if (GetOpenFileNameA(&ofn) == TRUE)
            {
                return std::string(ofn.lpstrFile);
            }
#endif
            return std::string();
        }

    };
} // namespace Uma_Engine