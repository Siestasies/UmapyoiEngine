#pragma once

#include "../Core/UITypes.h"
#include "rapidjson/document.h"

namespace Uma_UI
{
    /**
     * \class Canvas
     * \brief Root component for UI rendering. Defines how UI scales and sorts.
     */
    class Canvas
    {
    public:
        // Sorting
        int sortingOrder = 0;  // Higher values render on top

        // Resolution & Scaling
        Vec2 referenceResolution = Vec2(1920.0f, 1080.0f);
        Uma_UI::CanvasScaleMode scaleMode = Uma_UI::CanvasScaleMode::ScaleWithScreenSize;

        // Match width or height when scaling
        float matchWidthOrHeight = 0.5f;  // 0 = match width, 1 = match height, 0.5 = average

        // Runtime computed scale factor
        float scaleFactor = 1.0f;

        // Serialization
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

        void Deserialize(const rapidjson::Value& value)
        {
            sortingOrder = value["sortingOrder"].GetInt();

            const auto& refRes = value["referenceResolution"];
            referenceResolution.x = refRes["x"].GetFloat();
            referenceResolution.y = refRes["y"].GetFloat();

            scaleMode = static_cast<Uma_UI::CanvasScaleMode>(value["scaleMode"].GetInt());
            matchWidthOrHeight = value["matchWidthOrHeight"].GetFloat();

            scaleFactor = 1.0f;  // Will be recomputed in LayoutPass
        }
    };
}