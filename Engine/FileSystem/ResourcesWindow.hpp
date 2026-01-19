/*!
\file   ResourcesWindow.hpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Defines ImGui-based resource management window for engine asset loading and inspection.

Provides visual interface for textures, fonts, sounds, and shaders with drag-and-drop support.
Displays loaded resources in collapsible tables showing name, path, and metadata (texture ID,
font size, sound type, shader program ID). Handles file drops from OS with automatic type detection 
and popup dialogs for resource naming and configuration. Supports texture formats (png, jpg, jpeg, bmp),
font formats (ttf, otf), and audio formats (mp3, wav, ogg). Includes unload functionality for
manual resource cleanup. Shaders are displayed read-only for automated resource management.
Integrates with ResourcesManager for actual asset loading/unloading operations.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

// Engine/FileSystem/ResourcesWindow.hpp
#pragma once
#include "Systems/ResourcesManager.hpp"
#include "../EditorApp/imgui/imgui.h"
#include <filesystem>
#include <algorithm>
#include <string>
#include <string.h>

namespace Uma_Engine
{
    class ResourcesWindow
    {
    public:
        /**
         * \brief Default constructor initializing resource window state
         */
        ResourcesWindow()
            : m_ResourcesManager(nullptr)
            , m_TexturePopupJustOpened(false)
            , m_FontPopupJustOpened(false)
            , m_SoundPopupJustOpened(false)
            , m_ErrorPopupJustOpened(false)
        {
        }

        /**
         * \brief Sets the ResourcesManager instance for asset operations
         * \param rm Pointer to ResourcesManager
         */
        void SetResourcesManager(ResourcesManager* rm) { m_ResourcesManager = rm; }

        /**
         * \brief Renders the complete resources window including all tabs and popups
         */
        void Render()
        {
            if (!m_ResourcesManager) return;

            ImGui::Begin("Resources Manager");

            RenderTextures();
            RenderFonts();
            RenderSounds();
            RenderShaders();  // Read-only display for automated management
            RenderDropTarget();

            ImGui::End();

            RenderTexturePopup();
            RenderFontPopup();
            RenderSoundPopup();
            RenderErrorPopup();
        }

    private:
        ResourcesManager* m_ResourcesManager;

        // Texture popup state
        std::string m_PendingTexturePath;
        std::string m_PendingTextureName;
        char m_TextureNameBuffer[128];
        bool m_TexturePopupJustOpened;

        // Font popup state
        std::string m_PendingFontPath;
        std::string m_PendingFontName;
        char m_FontNameBuffer[128];
        int m_FontSize = 48;
        bool m_FontPopupJustOpened;

        // Sound popup state
        std::string m_PendingSoundPath;
        std::string m_PendingSoundName;
        char m_SoundNameBuffer[128];
        int m_SoundType = 0;
        bool m_SoundPopupJustOpened;

        // Error popup state
        std::string m_ErrorMsg;
        bool m_ErrorPopupJustOpened;

        /**
         * \brief Renders textures section with table displaying loaded texture assets
         */
        void RenderTextures()
        {
            if (ImGui::CollapsingHeader("Textures", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::Indent();

                const auto& textures = m_ResourcesManager->GetLoadedTextures();

                if (textures.empty())
                {
                    ImGui::TextDisabled("No textures loaded");
                }
                else
                {
                    if (ImGui::BeginTable("TexturesTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
                    {
                        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 150.0f);
                        ImGui::TableSetupColumn("Path", ImGuiTableColumnFlags_WidthStretch);
                        ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 60.0f);
                        ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 50.0f);
                        ImGui::TableHeadersRow();

                        std::string toDelete;
                        for (const auto& [name, texture] : textures)
                        {
                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0);
                            ImGui::Text("%s", name.c_str());

                            ImGui::TableSetColumnIndex(1);
                            ImGui::TextWrapped("%s", texture->filePath.c_str());

                            ImGui::TableSetColumnIndex(2);
                            ImGui::Text("%u", texture->tex_id);

                            ImGui::TableSetColumnIndex(3);
                            ImGui::PushID(name.c_str());
                            if (ImGui::SmallButton("X"))
                            {
                                toDelete = name;
                            }
                            ImGui::PopID();
                        }

                        ImGui::EndTable();

                        if (!toDelete.empty())
                        {
                            m_ResourcesManager->UnloadTexture(toDelete);
                        }
                    }
                }

                ImGui::Unindent();
            }
        }

        /**
         * \brief Renders fonts section with table displaying loaded font assets
         */
        void RenderFonts()
        {
            if (ImGui::CollapsingHeader("Fonts"))
            {
                ImGui::Indent();

                const auto& fonts = m_ResourcesManager->GetLoadedFonts();

                if (fonts.empty())
                {
                    ImGui::TextDisabled("No fonts loaded");
                }
                else
                {
                    if (ImGui::BeginTable("FontsTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
                    {
                        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 150.0f);
                        ImGui::TableSetupColumn("Path", ImGuiTableColumnFlags_WidthStretch);
                        ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, 60.0f);
                        ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 50.0f);
                        ImGui::TableHeadersRow();

                        std::string toDelete;
                        for (const auto& [name, fontData] : fonts)
                        {
                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0);
                            ImGui::Text("%s", name.c_str());

                            ImGui::TableSetColumnIndex(1);
                            ImGui::TextWrapped("%s", fontData.filePath.c_str());

                            ImGui::TableSetColumnIndex(2);
                            ImGui::Text("%u", fontData.fontSize);

                            ImGui::TableSetColumnIndex(3);
                            ImGui::PushID(name.c_str());
                            if (ImGui::SmallButton("X"))
                            {
                                toDelete = name;
                            }
                            ImGui::PopID();
                        }

                        ImGui::EndTable();

                        if (!toDelete.empty())
                        {
                            m_ResourcesManager->UnloadFont(toDelete);
                        }
                    }
                }

                ImGui::Unindent();
            }
        }

        /**
         * \brief Renders sounds section with table displaying loaded audio assets
         */
        void RenderSounds()
        {
            if (ImGui::CollapsingHeader("Sounds"))
            {
                ImGui::Indent();

                const auto& sounds = m_ResourcesManager->GetLoadedSounds();

                if (sounds.empty())
                {
                    ImGui::TextDisabled("No sounds loaded");
                }
                else
                {
                    if (ImGui::BeginTable("SoundsTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
                    {
                        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 150.0f);
                        ImGui::TableSetupColumn("Path", ImGuiTableColumnFlags_WidthStretch);
                        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                        ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 50.0f);
                        ImGui::TableHeadersRow();

                        std::string toDelete;
                        for (const auto& [name, soundInfo] : sounds)
                        {
                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0);
                            ImGui::Text("%s", name.c_str());

                            ImGui::TableSetColumnIndex(1);
                            ImGui::TextWrapped("%s", soundInfo.filePath.c_str());

                            ImGui::TableSetColumnIndex(2);
                            ImGui::Text("%s", soundInfo.type == SoundType::SFX ? "SFX" : "Music");

                            ImGui::TableSetColumnIndex(3);
                            ImGui::PushID(name.c_str());
                            if (ImGui::SmallButton("X"))
                            {
                                toDelete = name;
                            }
                            ImGui::PopID();
                        }

                        ImGui::EndTable();

                        if (!toDelete.empty())
                        {
                            m_ResourcesManager->UnloadSound(toDelete);
                        }
                    }
                }

                ImGui::Unindent();
            }
        }

        /**
         * \brief Renders shaders section with table displaying loaded shader programs (READ-ONLY)
         * This section is read-only for automated resource management
         */
        void RenderShaders()
        {
            if (ImGui::CollapsingHeader("Shaders"))
            {
                ImGui::Indent();

                // Info text for automated management
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.8f, 1.0f, 1.0f));
                ImGui::TextWrapped("Shaders are managed automatically by the engine");
                ImGui::PopStyleColor();
                ImGui::Spacing();

                const auto& shaders = m_ResourcesManager->GetLoadedShaders();

                if (shaders.empty())
                {
                    ImGui::TextDisabled("No shaders loaded");
                }
                else
                {
                    if (ImGui::BeginTable("ShadersTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
                    {
                        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 150.0f);
                        ImGui::TableSetupColumn("Vertex Shader", ImGuiTableColumnFlags_WidthStretch);
                        ImGui::TableSetupColumn("Fragment Shader", ImGuiTableColumnFlags_WidthStretch);
                        ImGui::TableSetupColumn("Program ID", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                        ImGui::TableHeadersRow();

                        for (const auto& [name, shader] : shaders)
                        {
                            ImGui::TableNextRow();
                            
                            ImGui::TableSetColumnIndex(0);
                            ImGui::Text("%s", name.c_str());

                            ImGui::TableSetColumnIndex(1);
                            ImGui::TextWrapped("%s", shader->vertexPath.c_str());

                            ImGui::TableSetColumnIndex(2);
                            ImGui::TextWrapped("%s", shader->fragmentPath.c_str());

                            ImGui::TableSetColumnIndex(3);
                            ImGui::Text("%u", shader->shaderProgramID);
                        }

                        ImGui::EndTable();
                    }

                    // Display total count
                    ImGui::Spacing();
                    ImGui::TextDisabled("Total: %zu shader(s)", shaders.size());
                }

                ImGui::Unindent();
            }
        }

        /**
         * \brief Renders drag-and-drop target area for file imports
         */
        void RenderDropTarget()
        {
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImVec2 dropTargetSize = ImVec2(ImGui::GetContentRegionAvail().x, 60);
            ImVec4 bgColor = ImVec4(0.2f, 0.2f, 0.25f, 1.0f);

            ImGui::PushStyleColor(ImGuiCol_ChildBg, bgColor);
            ImGui::BeginChild("DropTarget", dropTargetSize, true);

            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15);
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() * 0.5f - 100);
            ImGui::TextWrapped("Drop files here");

            ImGui::EndChild();
            ImGui::PopStyleColor();

            // Check if drop target was hovered
            if (ImGui::BeginDragDropTarget())
            {
                const ImGuiPayload* payload = ImGui::GetDragDropPayload();

                if (payload != nullptr)
                {
                    // Accept with the payload's type
                    if (const ImGuiPayload* accepted = ImGui::AcceptDragDropPayload(payload->DataType))
                    {
                        const char* droppedPath = (const char*)accepted->Data;

                        std::string filepath = std::string(droppedPath);
                        size_t assetsPos = filepath.find("Assets");

                        // SAFETY CHECK: Ensure file is in Assets folder to prevent crash
                        if (assetsPos != std::string::npos)
                        {
                            filepath = filepath.substr(assetsPos);
                            std::for_each(std::begin(filepath), std::end(filepath), [](char& c)
                                {
                                    c = (c == '\\') ? '/' : c;
                                });

                            std::cout << "[ResourcesWindow] Accepted drop: " << filepath << std::endl;
                            HandleResourceDrop(filepath);
                        }
                        else
                        {
                            std::cout << "[ResourcesWindow] Error: File must be in 'Assets' directory!" << std::endl;
                            m_ErrorMsg = "Invalid file location!\nFile must be inside the 'Assets' folder.";
                            m_ErrorPopupJustOpened = true;
                        }
                    }
                }

                ImGui::EndDragDropTarget();
            }
        }

        /**
         * \brief Handles dropped file by detecting type and triggering appropriate popup
         * \param filePath Path to dropped file relative to Assets folder
         */
        void HandleResourceDrop(const std::string& filePath)
        {
            namespace fs = std::filesystem;

            fs::path path(filePath);

            if (fs::is_directory(path))
            {
                std::cout << "[ResourcesWindow] Skipping directory: " << filePath << std::endl;
                return;
            }

            std::string ext = path.extension().string();
            std::string name = path.stem().string();
            std::transform(
                ext.begin(),
                ext.end(),
                ext.begin(),
                [](unsigned char c) { return static_cast<char>(std::tolower(c)); }
            );

            std::cout << "[ResourcesWindow] Processing: " << name << ext << std::endl;

            if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp")
            {
                std::cout << "[ResourcesWindow] Triggering texture popup" << std::endl;
                m_PendingTexturePath = filePath;
                m_PendingTextureName = name;
                strncpy(m_TextureNameBuffer, m_PendingTextureName.c_str(), sizeof(m_TextureNameBuffer));
                m_TextureNameBuffer[sizeof(m_TextureNameBuffer) - 1] = '\0';
                m_TexturePopupJustOpened = true;
            }
            else if (ext == ".ttf" || ext == ".otf")
            {
                std::cout << "[ResourcesWindow] Triggering font popup" << std::endl;
                m_PendingFontPath = filePath;
                m_PendingFontName = name;
                strncpy(m_FontNameBuffer, m_PendingFontName.c_str(), sizeof(m_FontNameBuffer));
                m_FontNameBuffer[sizeof(m_FontNameBuffer) - 1] = '\0';
                m_FontPopupJustOpened = true;
            }
            else if (ext == ".mp3" || ext == ".wav" || ext == ".ogg")
            {
                std::cout << "[ResourcesWindow] Triggering sound popup" << std::endl;
                m_PendingSoundPath = filePath;
                m_PendingSoundName = name;
                strncpy(m_SoundNameBuffer, m_PendingSoundName.c_str(), sizeof(m_SoundNameBuffer));
                m_SoundNameBuffer[sizeof(m_SoundNameBuffer) - 1] = '\0';
                m_SoundPopupJustOpened = true;
            }
            else if (ext == ".vert" || ext == ".frag" || ext == ".glsl")
            {
                std::cout << "[ResourcesWindow] Shader files are managed automatically" << std::endl;
                m_ErrorMsg = "Shader Management:\n\nShaders are loaded automatically by the engine.\nManual shader loading is not supported through drag-and-drop.";
                m_ErrorPopupJustOpened = true;
            }
            else
            {
                std::cout << "[ResourcesWindow] Unsupported type: " << ext << std::endl;
                m_ErrorMsg = "Unsupported file type: " + ext + "\n\nSupported types:\n- Textures: .png, .jpg, .jpeg, .bmp\n- Fonts: .ttf, .otf\n- Audio: .mp3, .wav, .ogg\n- Shaders: .vert, .frag, .glsl (auto-managed)";
                m_ErrorPopupJustOpened = true;
            }
        }

        /**
         * \brief Renders modal popup for texture import with name input
         */
        void RenderTexturePopup()
        {
            // Open popup if flag is set
            if (m_TexturePopupJustOpened)
            {
                std::cout << "[ResourcesWindow] Opening texture popup NOW" << std::endl;
                ImGui::OpenPopup("Add Texture");
                m_TexturePopupJustOpened = false;
            }

            // Define popup EVERY frame
            if (ImGui::BeginPopupModal("Add Texture", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("Texture Name:");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(150);
                ImGui::InputText("##texturename", m_TextureNameBuffer, IM_ARRAYSIZE(m_TextureNameBuffer));

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                if (ImGui::Button("Add Texture", ImVec2(120, 0)))
                {
                    m_PendingTextureName = std::string(m_TextureNameBuffer);

                    if (m_ResourcesManager->HasTexture(m_PendingTextureName))
                    {
                        std::cout << "[ResourcesWindow] Texture already exists: " << m_PendingTextureName << std::endl;
                    }
                    else
                    {
                        if (m_ResourcesManager->LoadTexture(m_PendingTextureName, m_PendingTexturePath))
                        {
                            std::cout << "[ResourcesWindow] Added texture: " << m_PendingTextureName << std::endl;
                        }
                        else
                        {
                            std::cout << "[ResourcesWindow] Failed to load texture: " << m_PendingTextureName << std::endl;
                        }
                    }
                    ImGui::CloseCurrentPopup();
                }

                ImGui::SameLine();

                if (ImGui::Button("Cancel", ImVec2(120, 0)))
                {
                    std::cout << "[ResourcesWindow] Texture popup cancelled" << std::endl;
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }
        }

        /**
         * \brief Renders modal popup for font import with name and size inputs
         */
        void RenderFontPopup()
        {
            // Open popup if flag is set
            if (m_FontPopupJustOpened)
            {
                std::cout << "[ResourcesWindow] Opening font popup NOW" << std::endl;
                ImGui::OpenPopup("Add Font");
                m_FontPopupJustOpened = false;
            }

            // Define popup EVERY frame
            if (ImGui::BeginPopupModal("Add Font", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("Font Name:");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(150);
                ImGui::InputText("##fontname", m_FontNameBuffer, IM_ARRAYSIZE(m_FontNameBuffer));

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::Text("Font Size:");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(150);
                ImGui::InputInt("##fontsize", &m_FontSize);

                if (m_FontSize < 8) m_FontSize = 8;
                if (m_FontSize > 256) m_FontSize = 256;

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                if (ImGui::Button("Add Font", ImVec2(120, 0)))
                {
                    m_PendingFontName = std::string(m_FontNameBuffer);

                    if (!m_ResourcesManager->HasFont(m_PendingFontName))
                    {
                        if (m_ResourcesManager->LoadFont(m_PendingFontName, m_PendingFontPath, static_cast<unsigned int>(m_FontSize)))
                        {
                            std::cout << "[ResourcesWindow] Successfully added font: " << m_PendingFontName << std::endl;
                        }
                        else
                        {
                            std::cout << "[ResourcesWindow] Failed to load font!" << std::endl;
                        }
                    }
                    else
                    {
                        std::cout << "[ResourcesWindow] Font already exists: " << m_PendingFontName << std::endl;
                    }
                    ImGui::CloseCurrentPopup();
                }

                ImGui::SameLine();

                if (ImGui::Button("Cancel", ImVec2(120, 0)))
                {
                    std::cout << "[ResourcesWindow] Font popup cancelled" << std::endl;
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }
        }

        /**
         * \brief Renders modal popup for sound import with name and type inputs
         */
        void RenderSoundPopup()
        {
            // Open popup if flag is set
            if (m_SoundPopupJustOpened)
            {
                std::cout << "[ResourcesWindow] Opening sound popup NOW" << std::endl;
                ImGui::OpenPopup("Add Sound");
                m_SoundPopupJustOpened = false;
            }

            // Define popup EVERY frame
            if (ImGui::BeginPopupModal("Add Sound", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("Sound Name:");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(150);
                ImGui::InputText("##soundname", m_SoundNameBuffer, IM_ARRAYSIZE(m_SoundNameBuffer));

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::Text("Sound Type:");
                ImGui::SameLine();
                const char* types[] = { "SFX (Sound Effect)", "Music (Background)" };
                ImGui::SetNextItemWidth(200);
                ImGui::Combo("##soundtype", &m_SoundType, types, 2);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                if (ImGui::Button("Add Sound", ImVec2(120, 0)))
                {
                    m_PendingSoundName = std::string(m_SoundNameBuffer);
                    SoundType type = (m_SoundType == 0) ? SoundType::SFX : SoundType::BGM;

                    if (!m_ResourcesManager->HasSound(m_PendingSoundName))
                    {
                        if (m_ResourcesManager->LoadSound(m_PendingSoundName, m_PendingSoundPath, type))
                        {
                            std::cout << "[ResourcesWindow] Successfully added sound: " << m_PendingSoundName << std::endl;
                        }
                        else
                        {
                            std::cout << "[ResourcesWindow] Failed to load sound!" << std::endl;
                        }
                    }
                    else
                    {
                        std::cout << "[ResourcesWindow] Sound already exists: " << m_PendingSoundName << std::endl;
                    }
                    ImGui::CloseCurrentPopup();
                }

                ImGui::SameLine();

                if (ImGui::Button("Cancel", ImVec2(120, 0)))
                {
                    std::cout << "[ResourcesWindow] Sound popup cancelled" << std::endl;
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }
        }

        /**
         * \brief Renders modal popup for error message
         */
        void RenderErrorPopup()
        {
            if (m_ErrorPopupJustOpened)
            {
                ImGui::OpenPopup("Error");
                m_ErrorPopupJustOpened = false;
            }

            // Center the popup
            ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

            if (ImGui::BeginPopupModal("Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::TextWrapped("%s", m_ErrorMsg.c_str());
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // Center the OK button
                float buttonWidth = 120.0f;
                float windowWidth = ImGui::GetWindowSize().x;
                ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);

                if (ImGui::Button("OK", ImVec2(buttonWidth, 0)))
                {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
        }
    };
}