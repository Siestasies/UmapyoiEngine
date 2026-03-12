/*!
\file   RectTransform.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Jedrek Lee Jing Wei (100%)
\par    E-mail: jedrekjingwei.lee@digipen.edu
\par    DigiPen login: jedrekjingwei.lee

\brief
Defines the RectTransform UI component for positioning and sizing.

This header provides the RectTransform class, which implements Unity-style
anchored rectangles with pivot points and hierarchical layout support.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "../Core/UITypes.h"
#include "rapidjson/document.h"

namespace Uma_UI
{
    /*!
     * \class RectTransform
     * \brief Unity-style anchored rectangle with pivot and size delta for UI layout.
     */
    class RectTransform
    {
    public:
        Vec2 anchorMin = Vec2(0.5f, 0.5f);
        Vec2 anchorMax = Vec2(0.5f, 0.5f);
        Vec2 pivot = Vec2(0.5f, 0.5f);
        Vec2 anchoredPosition = Vec2(0.0f, 0.0f);
        Vec2 sizeDelta = Vec2(100.0f, 100.0f);
        Uma_UI::Rect computedRect;
        bool isDirty = true;

        /*!
         * \brief Applies a predefined anchor preset to this transform.
         * \param preset The anchor preset configuration to apply.
         */
        void ApplyPreset(const Uma_UI::AnchorPreset& preset)
        {
            anchorMin = preset.anchorMin;
            anchorMax = preset.anchorMax;
            pivot = preset.pivot;
            isDirty = true;
        }

        /*!
         * \brief Checks if the transform stretches horizontally.
         * \return True if anchorMin.x != anchorMax.x.
         */
        bool IsStretchingHorizontal() const
        {
            return anchorMin.x != anchorMax.x;
        }

        /*!
         * \brief Checks if the transform stretches vertically.
         * \return True if anchorMin.y != anchorMax.y.
         */
        bool IsStretchingVertical() const
        {
            return anchorMin.y != anchorMax.y;
        }

        /*!
         * \brief Serializes rect transform properties to a JSON value.
         * \param value JSON value to populate.
         * \param allocator JSON document allocator.
         */
        void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const
        {
            value.SetObject();

            rapidjson::Value aMin(rapidjson::kObjectType);
            aMin.AddMember("x", anchorMin.x, allocator);
            aMin.AddMember("y", anchorMin.y, allocator);
            value.AddMember("anchorMin", aMin, allocator);

            rapidjson::Value aMax(rapidjson::kObjectType);
            aMax.AddMember("x", anchorMax.x, allocator);
            aMax.AddMember("y", anchorMax.y, allocator);
            value.AddMember("anchorMax", aMax, allocator);

            rapidjson::Value piv(rapidjson::kObjectType);
            piv.AddMember("x", pivot.x, allocator);
            piv.AddMember("y", pivot.y, allocator);
            value.AddMember("pivot", piv, allocator);

            rapidjson::Value pos(rapidjson::kObjectType);
            pos.AddMember("x", anchoredPosition.x, allocator);
            pos.AddMember("y", anchoredPosition.y, allocator);
            value.AddMember("anchoredPosition", pos, allocator);

            rapidjson::Value size(rapidjson::kObjectType);
            size.AddMember("x", sizeDelta.x, allocator);
            size.AddMember("y", sizeDelta.y, allocator);
            value.AddMember("sizeDelta", size, allocator);

            // computedRect is resolution-dependent — never persist it.
            // Always mark dirty on load so layout recomputes for the actual viewport.
            value.AddMember("isDirty", true, allocator);
        }

        /*!
         * \brief Deserializes rect transform properties from a JSON value.
         * \param value JSON value to read from.
         */
        void Deserialize(const rapidjson::Value& value)
        {
            const auto& aMin = value["anchorMin"];
            anchorMin.x = aMin["x"].GetFloat();
            anchorMin.y = aMin["y"].GetFloat();

            const auto& aMax = value["anchorMax"];
            anchorMax.x = aMax["x"].GetFloat();
            anchorMax.y = aMax["y"].GetFloat();

            const auto& piv = value["pivot"];
            pivot.x = piv["x"].GetFloat();
            pivot.y = piv["y"].GetFloat();

            const auto& pos = value["anchoredPosition"];
            anchoredPosition.x = pos["x"].GetFloat();
            anchoredPosition.y = pos["y"].GetFloat();

            const auto& size = value["sizeDelta"];
            sizeDelta.x = size["x"].GetFloat();
            sizeDelta.y = size["y"].GetFloat();

            // Always recompute on load — computedRect is viewport-dependent.
            computedRect = Uma_UI::Rect();
            isDirty = true;
        }
    };
}