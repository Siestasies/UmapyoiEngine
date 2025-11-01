#pragma once

#include "../Core/UITypes.h"
#include "rapidjson/document.h"

#include <string>

namespace Uma_UI
{
    /**
     * \class Text
     * \brief Renders text within a RectTransform bounds
     */
    class Text
    {
    public:
        // Content
        std::string text = "";
        std::string fontName = "";  // Font resource name

        // Styling
        float fontSize = 24.0f;
        Uma_UI::Colour colour = Uma_UI::Colour::Black();
        Uma_UI::TextAlignment alignment = Uma_UI::TextAlignment::Center;

        bool visible = true;

        // Serialization
        void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const
        {
            value.SetObject();

            value.AddMember("text",
                rapidjson::Value(text.c_str(), allocator),
                allocator);

            value.AddMember("fontName",
                rapidjson::Value(fontName.c_str(), allocator),
                allocator);

            value.AddMember("fontSize", fontSize, allocator);

            rapidjson::Value col(rapidjson::kObjectType);
            col.AddMember("r", colour.r, allocator);
            col.AddMember("g", colour.g, allocator);
            col.AddMember("b", colour.b, allocator);
            col.AddMember("a", colour.a, allocator);
            value.AddMember("colour", col, allocator);

            value.AddMember("alignment", static_cast<int>(alignment), allocator);
            value.AddMember("visible", visible, allocator);
        }

        void Deserialize(const rapidjson::Value& value)
        {
            text = value["text"].GetString();
            fontName = value["fontName"].GetString();
            fontSize = value["fontSize"].GetFloat();

            const auto& col = value["colour"];
            colour.r = col["r"].GetFloat();
            colour.g = col["g"].GetFloat();
            colour.b = col["b"].GetFloat();
            colour.a = col["a"].GetFloat();

            alignment = static_cast<Uma_UI::TextAlignment>(value["alignment"].GetInt());
            visible = value["visible"].GetBool();
        }
    };
}