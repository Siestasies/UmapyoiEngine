#pragma once

#include "../Core/UITypes.h"
#include "rapidjson/document.h"

namespace Uma_UI
{
    /**
     * \class RectTransform
     * \brief Unity-style anchored rectangle with pivot and size delta
     */
    class RectTransform
    {
    public:
        // Anchoring (normalized parent space [0,1])
        Vec2 anchorMin = Vec2(0.5f, 0.5f);  // Bottom-left anchor
        Vec2 anchorMax = Vec2(0.5f, 0.5f);  // Top-right anchor

        // Pivot (normalized local space [0,1])
        Vec2 pivot = Vec2(0.5f, 0.5f);  // 0,0 = bottom-left, 1,1 = top-right

        // Position offset from anchors (pixels or percentage)
        Vec2 anchoredPosition = Vec2(0.0f, 0.0f);

        // Size when not stretching (pixels)
        Vec2 sizeDelta = Vec2(100.0f, 100.0f);

        // Hierarchy
        Uma_ECS::Entity parent = static_cast<Uma_ECS::Entity>(-1);  // Invalid = root

        // Computed values (filled by LayoutPass)
        Uma_UI::Rect computedRect;  // Final NDC rectangle [-1,1]
        bool isDirty = true;  // Needs recomputation

        // Helper: Apply a preset
        void ApplyPreset(const Uma_UI::AnchorPreset& preset)
        {
            anchorMin = preset.anchorMin;
            anchorMax = preset.anchorMax;
            pivot = preset.pivot;
            isDirty = true;
        }

        // Helper: Check if stretching horizontally
        bool IsStretchingHorizontal() const
        {
            return anchorMin.x != anchorMax.x;
        }

        // Helper: Check if stretching vertically
        bool IsStretchingVertical() const
        {
            return anchorMin.y != anchorMax.y;
        }

        // Serialization
        void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const
        {
            value.SetObject();

            // Anchors
            rapidjson::Value aMin(rapidjson::kObjectType);
            aMin.AddMember("x", anchorMin.x, allocator);
            aMin.AddMember("y", anchorMin.y, allocator);
            value.AddMember("anchorMin", aMin, allocator);

            rapidjson::Value aMax(rapidjson::kObjectType);
            aMax.AddMember("x", anchorMax.x, allocator);
            aMax.AddMember("y", anchorMax.y, allocator);
            value.AddMember("anchorMax", aMax, allocator);

            // Pivot
            rapidjson::Value piv(rapidjson::kObjectType);
            piv.AddMember("x", pivot.x, allocator);
            piv.AddMember("y", pivot.y, allocator);
            value.AddMember("pivot", piv, allocator);

            // Position
            rapidjson::Value pos(rapidjson::kObjectType);
            pos.AddMember("x", anchoredPosition.x, allocator);
            pos.AddMember("y", anchoredPosition.y, allocator);
            value.AddMember("anchoredPosition", pos, allocator);

            // Size
            rapidjson::Value size(rapidjson::kObjectType);
            size.AddMember("x", sizeDelta.x, allocator);
            size.AddMember("y", sizeDelta.y, allocator);
            value.AddMember("sizeDelta", size, allocator);

            value.AddMember("parent", parent, allocator);
        }

        void Deserialize(const rapidjson::Value& value)
        {
            // Anchors
            const auto& aMin = value["anchorMin"];
            anchorMin.x = aMin["x"].GetFloat();
            anchorMin.y = aMin["y"].GetFloat();

            const auto& aMax = value["anchorMax"];
            anchorMax.x = aMax["x"].GetFloat();
            anchorMax.y = aMax["y"].GetFloat();

            // Pivot
            const auto& piv = value["pivot"];
            pivot.x = piv["x"].GetFloat();
            pivot.y = piv["y"].GetFloat();

            // Position
            const auto& pos = value["anchoredPosition"];
            anchoredPosition.x = pos["x"].GetFloat();
            anchoredPosition.y = pos["y"].GetFloat();

            // Size
            const auto& size = value["sizeDelta"];
            sizeDelta.x = size["x"].GetFloat();
            sizeDelta.y = size["y"].GetFloat();

            parent = value["parent"].GetUint();

            // Mark dirty for recomputation
            isDirty = true;
            computedRect = Uma_UI::Rect();
        }
    };
}