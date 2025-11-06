// Engine/FileSystem/ResourcesWindow.hpp
#pragma once
#include "Systems/ResourcesManager.hpp"
#include "imgui.h"
#include <filesystem>
#include <algorithm>
#include <string>
#include <string.h>

namespace Uma_Engine
{
    class ResourcesWindow
    {
    public:
        ResourcesWindow()
            : m_ResourcesManager(nullptr)
            , m_TexturePopupJustOpened(false)
            , m_FontPopupJustOpened(false)
            , m_SoundPopupJustOpened(false)
        {
        }

        void SetResourcesManager(ResourcesManager* rm) { m_ResourcesManager = rm; }

        void Render()
        {
            if (!m_ResourcesManager) return;

            ImGui::Begin("Resources Manager");

            RenderTextures();
            RenderFonts();
            RenderSounds();
            RenderDropTarget();

            ImGui::End();

            // CRITICAL: Render popups every frame, AFTER the main window
            RenderTexturePopup();
            RenderFontPopup();
            RenderSoundPopup();
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
                        filepath = filepath.substr(filepath.find("Assets"));
                        for_each(std::begin(filepath), std::end(filepath), [](char& c) 
                            {
                                c = (c == '\\') ? '/' : c;
                            });

                        std::cout << "[ResourcesWindow] Accepted drop: " << filepath << std::endl;
                        HandleResourceDrop(filepath);
                    }
                }

                ImGui::EndDragDropTarget();
            }
        }

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
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

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
            else
            {
                std::cout << "[ResourcesWindow] Unsupported type: " << ext << std::endl;
            }
        }

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
    };
}