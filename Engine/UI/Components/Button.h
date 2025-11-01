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
        Uma_UI::Colour normalColor = Uma_UI::Colour::White();
        Uma_UI::Colour hoverColor = Uma_UI::Colour(0.9f, 0.9f, 0.9f, 1.0f);
        Uma_UI::Colour pressedColor = Uma_UI::Colour(0.7f, 0.7f, 0.7f, 1.0f);
        Uma_UI::Colour disabledColor = Uma_UI::Colour(0.5f, 0.5f, 0.5f, 0.5f);

        // Callback (not serialized - set in code)
        Uma_UI::UICallback onClick = nullptr;

        // Runtime state tracking
        bool wasHoveredLastFrame = false;

        // Serialization
        void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const
        {
            value.SetObject();

            value.AddMember("interactable", interactable, allocator);

            // Normal color
            rapidjson::Value nCol(rapidjson::kObjectType);
            nCol.AddMember("r", normalColor.r, allocator);
            nCol.AddMember("g", normalColor.g, allocator);
            nCol.AddMember("b", normalColor.b, allocator);
            nCol.AddMember("a", normalColor.a, allocator);
            value.AddMember("normalColor", nCol, allocator);

            // Hover color
            rapidjson::Value hCol(rapidjson::kObjectType);
            hCol.AddMember("r", hoverColor.r, allocator);
            hCol.AddMember("g", hoverColor.g, allocator);
            hCol.AddMember("b", hoverColor.b, allocator);
            hCol.AddMember("a", hoverColor.a, allocator);
            value.AddMember("hoverColor", hCol, allocator);

            // Pressed color
            rapidjson::Value pCol(rapidjson::kObjectType);
            pCol.AddMember("r", pressedColor.r, allocator);
            pCol.AddMember("g", pressedColor.g, allocator);
            pCol.AddMember("b", pressedColor.b, allocator);
            pCol.AddMember("a", pressedColor.a, allocator);
            value.AddMember("pressedColor", pCol, allocator);

            // Disabled color
            rapidjson::Value dCol(rapidjson::kObjectType);
            dCol.AddMember("r", disabledColor.r, allocator);
            dCol.AddMember("g", disabledColor.g, allocator);
            dCol.AddMember("b", disabledColor.b, allocator);
            dCol.AddMember("a", disabledColor.a, allocator);
            value.AddMember("disabledColor", dCol, allocator);
        }

        void Deserialize(const rapidjson::Value& value)
        {
            interactable = value["interactable"].GetBool();

            // Normal color
            const auto& nCol = value["normalColor"];
            normalColor.r = nCol["r"].GetFloat();
            normalColor.g = nCol["g"].GetFloat();
            normalColor.b = nCol["b"].GetFloat();
            normalColor.a = nCol["a"].GetFloat();

            // Hover color
            const auto& hCol = value["hoverColor"];
            hoverColor.r = hCol["r"].GetFloat();
            hoverColor.g = hCol["g"].GetFloat();
            hoverColor.b = hCol["b"].GetFloat();
            hoverColor.a = hCol["a"].GetFloat();

            // Pressed color
            const auto& pCol = value["pressedColor"];
            pressedColor.r = pCol["r"].GetFloat();
            pressedColor.g = pCol["g"].GetFloat();
            pressedColor.b = pCol["b"].GetFloat();
            pressedColor.a = pCol["a"].GetFloat();

            // Disabled color
            const auto& dCol = value["disabledColor"];
            disabledColor.r = dCol["r"].GetFloat();
            disabledColor.g = dCol["g"].GetFloat();
            disabledColor.b = dCol["b"].GetFloat();
            disabledColor.a = dCol["a"].GetFloat();

            // Reset runtime state
            currentState = interactable ? Uma_UI::ButtonState::Normal : Uma_UI::ButtonState::Disabled;
            wasHoveredLastFrame = false;
            onClick = nullptr;  // Must be set programmatically
        }
    };
}