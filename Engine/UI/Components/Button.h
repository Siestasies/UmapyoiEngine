#pragma once

#include "../Core/UITypes.h"
#include "rapidjson/document.h"

namespace Uma_UI
{
    /**
     * \class Button
     * \brief Interactive button with state-based color transitions
     */
    class Button
    {
    public:
        // Interaction
        bool interactable = true;
        Uma_UI::ButtonState currentState = Uma_UI::ButtonState::Normal;

        // Visual states
        Uma_UI::Colour normalColour = Uma_UI::Colour::White();
        Uma_UI::Colour hoverColour = Uma_UI::Colour(0.9f, 0.9f, 0.9f, 1.0f);
        Uma_UI::Colour pressedColour = Uma_UI::Colour(0.7f, 0.7f, 0.7f, 1.0f);
        Uma_UI::Colour disabledColour = Uma_UI::Colour(0.5f, 0.5f, 0.5f, 0.5f);

        // Callback (not serialized - set in code)
        Uma_UI::UICallback onClick = nullptr;

        std::string functionName = {};

        // Runtime state tracking
        bool wasHoveredLastFrame = false;

        // Serialization
        void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const
        {
            value.SetObject();

            value.AddMember("interactable", interactable, allocator);

            // Normal color
            rapidjson::Value nCol(rapidjson::kObjectType);
            nCol.AddMember("r", normalColour.r, allocator);
            nCol.AddMember("g", normalColour.g, allocator);
            nCol.AddMember("b", normalColour.b, allocator);
            nCol.AddMember("a", normalColour.a, allocator);
            value.AddMember("normalColour", nCol, allocator);

            // Hover color
            rapidjson::Value hCol(rapidjson::kObjectType);
            hCol.AddMember("r", hoverColour.r, allocator);
            hCol.AddMember("g", hoverColour.g, allocator);
            hCol.AddMember("b", hoverColour.b, allocator);
            hCol.AddMember("a", hoverColour.a, allocator);
            value.AddMember("hoverColour", hCol, allocator);

            // Pressed color
            rapidjson::Value pCol(rapidjson::kObjectType);
            pCol.AddMember("r", pressedColour.r, allocator);
            pCol.AddMember("g", pressedColour.g, allocator);
            pCol.AddMember("b", pressedColour.b, allocator);
            pCol.AddMember("a", pressedColour.a, allocator);
            value.AddMember("pressedColour", pCol, allocator);

            // Disabled color
            rapidjson::Value dCol(rapidjson::kObjectType);
            dCol.AddMember("r", disabledColour.r, allocator);
            dCol.AddMember("g", disabledColour.g, allocator);
            dCol.AddMember("b", disabledColour.b, allocator);
            dCol.AddMember("a", disabledColour.a, allocator);
            value.AddMember("disabledColour", dCol, allocator);

            value.AddMember("functionName", rapidjson::Value(functionName.c_str(), allocator), allocator);
        }

        void Deserialize(const rapidjson::Value& value)
        {
            interactable = value["interactable"].GetBool();

            // Normal color
            const auto& nCol = value["normalColour"];
            normalColour.r = nCol["r"].GetFloat();
            normalColour.g = nCol["g"].GetFloat();
            normalColour.b = nCol["b"].GetFloat();
            normalColour.a = nCol["a"].GetFloat();

            // Hover color
            const auto& hCol = value["hoverColour"];
            hoverColour.r = hCol["r"].GetFloat();
            hoverColour.g = hCol["g"].GetFloat();
            hoverColour.b = hCol["b"].GetFloat();
            hoverColour.a = hCol["a"].GetFloat();

            // Pressed color
            const auto& pCol = value["pressedColour"];
            pressedColour.r = pCol["r"].GetFloat();
            pressedColour.g = pCol["g"].GetFloat();
            pressedColour.b = pCol["b"].GetFloat();
            pressedColour.a = pCol["a"].GetFloat();

            // Disabled color
            const auto& dCol = value["disabledColour"];
            disabledColour.r = dCol["r"].GetFloat();
            disabledColour.g = dCol["g"].GetFloat();
            disabledColour.b = dCol["b"].GetFloat();
            disabledColour.a = dCol["a"].GetFloat();

            if (value.HasMember("functionName"))
                functionName = value["functionName"].GetString();

            // Reset runtime state
            currentState = interactable ? Uma_UI::ButtonState::Normal : Uma_UI::ButtonState::Disabled;
            wasHoveredLastFrame = false;
            onClick = nullptr;  // Must be set programmatically
        }
    };
}