#pragma once

#include "../Core/UITypes.h"
#include "rapidjson/document.h"

#include <string>

namespace Uma_UI
{
    /**
     * \class Image
     * \brief Renders a textured sprite within a RectTransform
     */
    class Image
    {
    public:
        // Texture
        std::string textureName = "";

        // Visual
        Uma_UI::Colour color = Uma_UI::Colour::White();  // Color tint
        bool visible = true;

        // Serialization
        void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const
        {
            value.SetObject();

            value.AddMember("textureName",
                rapidjson::Value(textureName.c_str(), allocator),
                allocator);

            rapidjson::Value col(rapidjson::kObjectType);
            col.AddMember("r", color.r, allocator);
            col.AddMember("g", color.g, allocator);
            col.AddMember("b", color.b, allocator);
            col.AddMember("a", color.a, allocator);
            value.AddMember("color", col, allocator);

            value.AddMember("visible", visible, allocator);
        }

        void Deserialize(const rapidjson::Value& value)
        {
            textureName = value["textureName"].GetString();

            const auto& col = value["color"];
            color.r = col["r"].GetFloat();
            color.g = col["g"].GetFloat();
            color.b = col["b"].GetFloat();
            color.a = col["a"].GetFloat();

            visible = value["visible"].GetBool();
        }
    };
}