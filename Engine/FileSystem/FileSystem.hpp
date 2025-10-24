#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include "imgui.h"

namespace fs = std::filesystem;

struct FileEntry {
    std::string name;
    std::string path;
    bool is_directory;
    uintmax_t size;
    fs::file_time_type last_modified;

    FileEntry(const fs::directory_entry& entry)
        : name(entry.path().filename().string())
        , path(entry.path().string())
        , is_directory(entry.is_directory())
        , size(entry.is_directory() ? 0 : entry.file_size())
        , last_modified(entry.last_write_time())
    {}
};

enum class SortMode {
    Name,
    Size,
    Type,
    Modified
};

class FileBrowser {
public:
    FileBrowser(const std::string& root_path = ".")
        : m_current_path(fs::absolute(root_path))
        , m_sort_mode(SortMode::Name)
        , m_sort_ascending(true)
    {
        m_filter[0] = '\0';
        RefreshDirectory();
    }

    void Render() {
        ImGui::Begin("File Browser");

        RenderNavigationBar();
        ImGui::Separator();

        RenderFilterBar();
        ImGui::Separator();

        RenderFileList();

        ImGui::End();
    }

    // Get the currently selected file path (if any)
    const std::string& GetSelectedPath() const { return m_selected_path; }

    // Check if a file was double-clicked (for opening)
    bool WasFileDoubleClicked() const { return m_file_double_clicked; }

private:
    fs::path m_current_path;
    std::vector<FileEntry> m_entries;
    std::string m_selected_path;
    char m_filter[256];
    SortMode m_sort_mode;
    bool m_sort_ascending;
    bool m_file_double_clicked = false;

    void RefreshDirectory() {
        m_entries.clear();

        try {
            for (const auto& entry : fs::directory_iterator(m_current_path)) {
                m_entries.emplace_back(entry);
            }
            SortEntries();
        }
        catch (const fs::filesystem_error& e) {
            // Handle errors (permissions, invalid path, etc.)
        }
    }

    void SortEntries() {
        std::sort(m_entries.begin(), m_entries.end(), [this](const FileEntry& a, const FileEntry& b) {
            // Directories always first
            if (a.is_directory != b.is_directory)
                return a.is_directory;

            bool result = false;
            switch (m_sort_mode) {
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

            return m_sort_ascending ? result : !result;
            });
    }

    void RenderNavigationBar() {
        // Up directory button
        if (ImGui::Button("^ Up") && m_current_path.has_parent_path()) {
            m_current_path = m_current_path.parent_path();
            RefreshDirectory();
        }

        ImGui::SameLine();

        // Current path display (clickable breadcrumbs)
        ImGui::Text("Path:");
        ImGui::SameLine();

        std::string path_str = m_current_path.string();
        ImGui::TextWrapped("%s", path_str.c_str());

        // Quick navigation buttons
        ImGui::SameLine();
        if (ImGui::Button("Refresh")) {
            RefreshDirectory();
        }
    }

    void RenderFilterBar() {
        ImGui::SetNextItemWidth(200);
        if (ImGui::InputText("Filter", m_filter, IM_ARRAYSIZE(m_filter))) {
            // Filter updated
        }

        ImGui::SameLine();

        // Sort mode selector
        const char* sort_modes[] = { "Name", "Size", "Type", "Modified" };
        int current_mode = static_cast<int>(m_sort_mode);
        ImGui::SetNextItemWidth(100);
        if (ImGui::Combo("Sort", &current_mode, sort_modes, IM_ARRAYSIZE(sort_modes))) {
            m_sort_mode = static_cast<SortMode>(current_mode);
            SortEntries();
        }

        ImGui::SameLine();
        if (ImGui::Button(m_sort_ascending ? "Asc" : "Desc")) {
            m_sort_ascending = !m_sort_ascending;
            SortEntries();
        }
    }

    void RenderFileList() {
        m_file_double_clicked = false;

        ImGui::BeginChild("FileList", ImVec2(0, 0), true);

        for (const auto& entry : m_entries) {
            // Apply filter
            if (m_filter[0] != '\0' &&
                entry.name.find(m_filter) == std::string::npos) {
                continue;
            }

            // Icon based on type
            const char* icon = entry.is_directory ? "[DIR]" : "[FILE]";
            std::string label = std::string(icon) + " " + entry.name;

            bool is_selected = (m_selected_path == entry.path);

            if (ImGui::Selectable(label.c_str(), is_selected, ImGuiSelectableFlags_AllowDoubleClick)) {
                m_selected_path = entry.path;

                // Handle double-click
                if (ImGui::IsMouseDoubleClicked(0)) {
                    if (entry.is_directory) {
                        m_current_path = entry.path;
                        RefreshDirectory();
                        m_selected_path.clear();
                    }
                    else {
                        m_file_double_clicked = true;
                    }
                }
            }

            // Drag and drop source
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                // Store the full path in the payload
                ImGui::SetDragDropPayload("FILE_PATH", entry.path.c_str(), entry.path.size() + 1);
                ImGui::Text("%s", entry.name.c_str());
                ImGui::EndDragDropSource();
            }

            // Tooltip with details
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::Text("Path: %s", entry.path.c_str());
                if (!entry.is_directory) {
                    ImGui::Text("Size: %llu bytes", entry.size);
                }
                ImGui::EndTooltip();
            }
        }

        ImGui::EndChild();
    }
};

// Usage example:
/*
class MyEditor {
    FileBrowser m_file_browser;

    void Update() {
        m_file_browser.Render();

        // Check if a file was double-clicked to open it
        if (m_file_browser.WasFileDoubleClicked()) {
            std::string path = m_file_browser.GetSelectedPath();
            LoadAsset(path);
        }

        // In your viewport or other drop target:
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE_PATH")) {
                const char* dropped_path = static_cast<const char*>(payload->Data);
                HandleDroppedFile(dropped_path);
            }
            ImGui::EndDragDropTarget();
        }
    }
};
*/