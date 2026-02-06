/*!
\file   ResourcesWindow.hpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\co-author  Javier Chua Dong Qing (Changed texture and font + add shader and prefab)
\par        E-mail: javierdongqing.chua@digipen.edu
\par        DigiPen login: javierdongqing.chua

\brief
Defines ImGui-based resource management window for engine asset loading and inspection.

Provides visual interface for textures, fonts, sounds, and shaders.
Displays loaded resources in collapsible tables showing name, path, and metadata (texture ID,
font size, sound type, shader program ID).

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

// Engine/FileSystem/ResourcesWindow.hpp
#pragma once
#include "Systems/ResourcesManager.hpp"
#include "../EditorApp/imgui/imgui.h"
#include <string>

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
        {
        }

        /**
         * \brief Sets the ResourcesManager instance for asset operations
         * \param rm Pointer to ResourcesManager
         */
        void SetResourcesManager(ResourcesManager* rm) { m_ResourcesManager = rm; }

        /**
         * \brief Renders the complete resources window including all tabs
         */
        void Render()
        {
            if (!m_ResourcesManager) return;

            ImGui::Begin("Resources Manager");

            RenderTextures();
            RenderFonts();
            RenderSounds();
            RenderPrefabs();
            RenderShaders();  // Read-only display for automated management

            ImGui::End();
        }

    private:
        ResourcesManager* m_ResourcesManager;

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
                    if (ImGui::BeginTable("TexturesTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
                    {
                        ImGui::TableSetupColumn("Path", ImGuiTableColumnFlags_WidthStretch);
                        ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 60.0f);
                        ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 50.0f);
                        ImGui::TableHeadersRow();

                        std::string toDelete;
                        for (const auto& [path, texture] : textures)
                        {
                            ImGui::TableNextRow();

                            ImGui::TableSetColumnIndex(0);
                            ImGui::TextWrapped("%s", path.c_str());

                            ImGui::TableSetColumnIndex(1);
                            ImGui::Text("%u", texture->tex_id);

                            ImGui::TableSetColumnIndex(2);
                            ImGui::PushID(path.c_str());
                            if (ImGui::SmallButton("X"))
                            {
                                toDelete = path;
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
                    if (ImGui::BeginTable("FontsTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
                    {
                        ImGui::TableSetupColumn("Path", ImGuiTableColumnFlags_WidthStretch);
                        ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, 60.0f);
                        ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 50.0f);
                        ImGui::TableHeadersRow();

                        std::string toDelete;
                        for (const auto& [path, fontData] : fonts)
                        {
                            ImGui::TableNextRow();

                            ImGui::TableSetColumnIndex(0);
                            ImGui::TextWrapped("%s", path.c_str());

                            ImGui::TableSetColumnIndex(1);
                            ImGui::Text("%u", fontData.fontSize);

                            ImGui::TableSetColumnIndex(2);
                            ImGui::PushID(path.c_str());
                            if (ImGui::SmallButton("X"))
                            {
                                toDelete = path;
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
         * \brief Renders prefabs section with table
         */
        void RenderPrefabs()
        {
            if (ImGui::CollapsingHeader("Prefabs"))
            {
                ImGui::Indent();

                const auto& prefabs = m_ResourcesManager->GetLoadedPrefabs();

                if (prefabs.empty())
                {
                    ImGui::TextDisabled("No prefabs loaded");
                }
                else
                {
                    if (ImGui::BeginTable("PrefabsTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
                    {
                        ImGui::TableSetupColumn("Path", ImGuiTableColumnFlags_WidthStretch);
                        ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthFixed, 60.0f);
                        ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 50.0f);
                        ImGui::TableHeadersRow();

                        std::string toDelete;
                        for (const auto& [path, doc] : prefabs)
                        {
                            ImGui::TableNextRow();

                            ImGui::TableSetColumnIndex(0);
                            ImGui::TextWrapped("%s", path.c_str());

                            ImGui::TableSetColumnIndex(1);
                            ImGui::Text("%s", (doc && !doc->HasParseError()) ? "Valid" : "Error");

                            ImGui::TableSetColumnIndex(2);
                            ImGui::PushID(path.c_str());
                            if (ImGui::SmallButton("X"))
                            {
                                toDelete = path;
                            }
                            ImGui::PopID();
                        }

                        ImGui::EndTable();

                        if (!toDelete.empty())
                        {
                            m_ResourcesManager->UnloadPrefab(toDelete);
                        }
                    }
                }

                ImGui::Unindent();
            }
        }
    };
}