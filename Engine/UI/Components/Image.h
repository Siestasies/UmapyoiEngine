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
    /*!
     * \class Image
     * \brief Renders a textured sprite within a RectTransform bounds.
     */
    class Image
    {
    public:
        std::string textureName = "";
        int sortingOrder = 0;
        Uma_UI::Color color = Uma_UI::Color::White();
        bool visible = true;


        // runtime variable
        std::shared_ptr<Uma_Engine::Texture> texture = nullptr;

        /*!
         * \brief Serializes image properties to a JSON value.
         * \param value JSON value to populate.
         * \param allocator JSON document allocator.
         */
        void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const
        {
            value.SetObject();
            value.AddMember("textureName", rapidjson::Value(textureName.c_str(), allocator), allocator);

            value.AddMember("sortingOrder", sortingOrder, allocator);

            rapidjson::Value col(rapidjson::kObjectType);
            col.AddMember("r", color.r, allocator);
            col.AddMember("g", color.g, allocator);
            col.AddMember("b", color.b, allocator);
            col.AddMember("a", color.a, allocator);
            value.AddMember("color", col, allocator);

            value.AddMember("visible", visible, allocator);
        }

        /*!
         * \brief Deserializes image properties from a JSON value.
         * \param value JSON value to read from.
         */
        void Deserialize(const rapidjson::Value& value)
        {
            textureName = value["textureName"].GetString();

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
        }
    };
}