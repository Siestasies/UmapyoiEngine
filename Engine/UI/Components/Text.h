/*!
\file   Text.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Jedrek Lee Jing Wei (100%)
\par    E-mail: jedrekjingwei.lee@digipen.edu
\par    DigiPen login: jedrekjingwei.lee

\brief
Defines the Text UI component for rendering text.

This header provides the Text class, which handles text content, font selection,
sizing, alignment, and color properties for UI text rendering.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "../Core/UITypes.h"
#include "rapidjson/document.h"
#include <string>

namespace Uma_UI
{
    /*!
     * \class Text
     * \brief Renders text within RectTransform bounds with styling options.
     */
    class Text
    {
    public:
        std::string text = "";
        std::string fontName = "";
        int sortingOrder = 0;
        float fontSize = 24.0f;
        Uma_UI::Color color = Uma_UI::Color::Black();
        Uma_UI::TextAlignment alignment = Uma_UI::TextAlignment::Center;
        bool visible = true;


        /*!
         * \brief Serializes text properties to a JSON value.
         * \param value JSON value to populate.
         * \param allocator JSON document allocator.
         */
        void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const
        {
            value.SetObject();
            value.AddMember("text", rapidjson::Value(text.c_str(), allocator), allocator);
            value.AddMember("fontName", rapidjson::Value(fontName.c_str(), allocator), allocator);
            value.AddMember("sortingOrder", sortingOrder, allocator);
            value.AddMember("fontSize", fontSize, allocator);

            rapidjson::Value col(rapidjson::kObjectType);
            col.AddMember("r", color.r, allocator);
            col.AddMember("g", color.g, allocator);
            col.AddMember("b", color.b, allocator);
            col.AddMember("a", color.a, allocator);
            value.AddMember("colour", col, allocator);

            value.AddMember("alignment", static_cast<int>(alignment), allocator);
            value.AddMember("visible", visible, allocator);
        }

        /*!
         * \brief Deserializes text properties from a JSON value.
         * \param value JSON value to read from.
         */
        void Deserialize(const rapidjson::Value& value)
        {
            text = value["text"].GetString();
            fontName = value["fontName"].GetString();

            if (value.HasMember("sortingOrder"))
            {
                sortingOrder = value["sortingOrder"].GetInt();
            }

            fontSize = value["fontSize"].GetFloat();

            const auto& col = value["colour"];
            color.r = col["r"].GetFloat();
            color.g = col["g"].GetFloat();
            color.b = col["b"].GetFloat();
            color.a = col["a"].GetFloat();

            alignment = static_cast<Uma_UI::TextAlignment>(value["alignment"].GetInt());
            visible = value["visible"].GetBool();
        }
    };
}