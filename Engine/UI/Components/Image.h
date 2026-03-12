/*!
\file   Image.h
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A
\par    Software Engineering Project 4

\author Jedrek Lee Jing Wei (100%)
\par    E-mail: jedrekjingwei.lee@digipen.edu
\par    DigiPen login: jedrekjingwei.lee

\brief
Defines the Image UI component for rendering textured sprites.

This header provides the Image class, which stores texture references,
color tinting, and visibility settings for UI sprites.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "../Core/UITypes.h"
#include "rapidjson/document.h"
#include <string>

namespace Uma_UI
{
    enum class FillDirection
    {
        None = 0,
        LeftToRight,
        RightToLeft,
        TopToBottom,
        BottomToTop
    };

    /*!
     * \class Image
     * \brief Renders a textured sprite within a RectTransform bounds.
     */
    class Image
    {
    public:
        std::string texturePath = "";
        int sortingOrder = 0;
        Uma_UI::Color color = Uma_UI::Color::White();
        bool visible = true;
        std::shared_ptr<Uma_Engine::Texture> texture = nullptr;

        FillDirection fillDirection = FillDirection::None;
        float fillAmount = 1.0f;  // 0.0 to 1.0 (0 = empty, 1 = full)

        // Spritesheet support (mirrors Sprite component)
        Vec2 spriteSheetGrid = Vec2(1.0f, 1.0f);  // Total columns and rows (default = full texture)
        Vec2 spriteCell = Vec2(0.0f, 0.0f);   // Which cell to render (col, row)
        Vec2 spriteOffset = Vec2(0.0f, 0.0f);   // Additional UV offset within a cell

        bool change = false;

        /*!
         * \brief Calculates UV coordinates for the current sprite cell.
         * \param uvOffset [Out] Starting UV coordinate (top-left) of the cell in [0,1] space.
         * \param uvSize   [Out] Width and height of the cell in UV space.
         *
         * When spriteSheetGrid is (1,1) the entire texture is used (uvOffset=(0,0), uvSize=(1,1)).
         */
        void GetUVs(Vec2& uvOffset, Vec2& uvSize) const
        {
            uvSize.x = 1.0f / spriteSheetGrid.x;
            uvSize.y = 1.0f / spriteSheetGrid.y;

            uvOffset.x = spriteCell.x * uvSize.x + spriteOffset.x;
            uvOffset.y = spriteCell.y * uvSize.y + spriteOffset.y;
        }

        /*!
         * \brief Advances to the next cell in row-major order, wrapping around.
         * \return True if the animation looped back to cell (0,0).
         */
        bool AdvanceFrame()
        {
            spriteCell.x += 1.0f;
            if (spriteCell.x >= spriteSheetGrid.x)
            {
                spriteCell.x = 0.0f;
                spriteCell.y += 1.0f;
                if (spriteCell.y >= spriteSheetGrid.y)
                {
                    spriteCell.y = 0.0f;
                    return true;  // looped
                }
            }
            return false;
        }

        /*!
         * \brief Sets the active cell by a flat frame index (row-major).
         * \param frameIndex Zero-based frame index.
         */
        void SetFrame(int frameIndex)
        {
            int cols = static_cast<int>(spriteSheetGrid.x);
            if (cols < 1) cols = 1;
            spriteCell.x = static_cast<float>(frameIndex % cols);
            spriteCell.y = static_cast<float>(frameIndex / cols);
        }

        /*!
         * \brief Returns the current frame index (row-major).
         */
        int GetFrame() const
        {
            int cols = static_cast<int>(spriteSheetGrid.x);
            if (cols < 1) cols = 1;
            return static_cast<int>(spriteCell.y) * cols + static_cast<int>(spriteCell.x);
        }

        /*!
         * \brief Serializes image properties to a JSON value.
         * \param value JSON value to populate.
         * \param allocator JSON document allocator.
         */
        void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const
        {
            value.SetObject();
            value.AddMember("texturePath", rapidjson::Value(texturePath.c_str(), allocator), allocator);

            value.AddMember("sortingOrder", sortingOrder, allocator);

            rapidjson::Value col(rapidjson::kObjectType);
            col.AddMember("r", color.r, allocator);
            col.AddMember("g", color.g, allocator);
            col.AddMember("b", color.b, allocator);
            col.AddMember("a", color.a, allocator);
            value.AddMember("color", col, allocator);

            value.AddMember("visible", visible, allocator);
            value.AddMember("fillDirection", static_cast<int>(fillDirection), allocator);
            value.AddMember("fillAmount", fillAmount, allocator);

            rapidjson::Value sheetGrid(rapidjson::kObjectType);
            sheetGrid.AddMember("x", spriteSheetGrid.x, allocator);
            sheetGrid.AddMember("y", spriteSheetGrid.y, allocator);
            value.AddMember("spriteSheetGrid", sheetGrid, allocator);

            rapidjson::Value cell(rapidjson::kObjectType);
            cell.AddMember("x", spriteCell.x, allocator);
            cell.AddMember("y", spriteCell.y, allocator);
            value.AddMember("spriteCell", cell, allocator);

            rapidjson::Value offset(rapidjson::kObjectType);
            offset.AddMember("x", spriteOffset.x, allocator);
            offset.AddMember("y", spriteOffset.y, allocator);
            value.AddMember("spriteOffset", offset, allocator);
        }

        /*!
         * \brief Deserializes image properties from a JSON value.
         * \param value JSON value to read from.
         */
        void Deserialize(const rapidjson::Value& value)
        {
            if (value.HasMember("texturePath"))
            {
                texturePath = value["texturePath"].GetString();
            }

            if (value.HasMember("sortingOrder"))
            {
                sortingOrder = value["sortingOrder"].GetInt();
            }

            const auto& col = value["color"];
            color.r = col["r"].GetFloat();
            color.g = col["g"].GetFloat();
            color.b = col["b"].GetFloat();
            color.a = col["a"].GetFloat();

            visible = value["visible"].GetBool();

            if (value.HasMember("fillDirection"))
            {
                fillDirection = static_cast<FillDirection>(value["fillDirection"].GetInt());
            }

            if (value.HasMember("fillAmount"))
            {
                fillAmount = value["fillAmount"].GetFloat();
            }

            if (value.HasMember("spriteSheetGrid"))
            {
                const auto& g = value["spriteSheetGrid"];
                spriteSheetGrid.x = g["x"].GetFloat();
                spriteSheetGrid.y = g["y"].GetFloat();
            }

            if (value.HasMember("spriteCell"))
            {
                const auto& c = value["spriteCell"];
                spriteCell.x = c["x"].GetFloat();
                spriteCell.y = c["y"].GetFloat();
            }

            if (value.HasMember("spriteOffset"))
            {
                const auto& o = value["spriteOffset"];
                spriteOffset.x = o["x"].GetFloat();
                spriteOffset.y = o["y"].GetFloat();
            }
        }
    };
}