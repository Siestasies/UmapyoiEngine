/*!
\file   Image.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

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

        bool change = false;

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
        }
    };
}