/*!
\file   Canvas.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author CSD2401 Jedrek Lee Jing Wei (100%)
\par    E-mail: jedrekjingwei.lee@digipen.edu
\par    DigiPen login: jedrekjingwei.lee

\brief
Defines the Canvas UI component for root-level UI configuration.

This header provides the Canvas class, which controls UI scaling behavior,
sorting order, and reference resolution for responsive layouts.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "../Core/UITypes.h"
#include "rapidjson/document.h"

namespace Uma_UI
{
    /*!
     * \class Canvas
     * \brief Root component for UI rendering that defines scaling and sorting behavior.
     */
    class Canvas
    {
    public:
        int sortingOrder = 0;
        Vec2 referenceResolution = Vec2(1920.0f, 1080.0f);
        Uma_UI::CanvasScaleMode scaleMode = Uma_UI::CanvasScaleMode::ScaleWithScreenSize;
        float matchWidthOrHeight = 0.5f;
        float scaleFactor = 1.0f;

        /*!
         * \brief Serializes canvas properties to a JSON value.
         * \param value JSON value to populate.
         * \param allocator JSON document allocator.
         */
        void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const
        {
            value.SetObject();
            value.AddMember("sortingOrder", sortingOrder, allocator);

            rapidjson::Value refRes(rapidjson::kObjectType);
            refRes.AddMember("x", referenceResolution.x, allocator);
            refRes.AddMember("y", referenceResolution.y, allocator);
            value.AddMember("referenceResolution", refRes, allocator);

            value.AddMember("scaleMode", static_cast<int>(scaleMode), allocator);
            value.AddMember("matchWidthOrHeight", matchWidthOrHeight, allocator);
        }

        /*!
         * \brief Deserializes canvas properties from a JSON value.
         * \param value JSON value to read from.
         */
        void Deserialize(const rapidjson::Value& value)
        {
            sortingOrder = value["sortingOrder"].GetInt();

            const auto& refRes = value["referenceResolution"];
            referenceResolution.x = refRes["x"].GetFloat();
            referenceResolution.y = refRes["y"].GetFloat();

            scaleMode = static_cast<Uma_UI::CanvasScaleMode>(value["scaleMode"].GetInt());
            matchWidthOrHeight = value["matchWidthOrHeight"].GetFloat();

            scaleFactor = 1.0f;
        }
    };
}