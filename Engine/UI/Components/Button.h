/*!
\file   Button.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author CSD2401 Jedrek Lee Jing Wei (100%)
\par    E-mail: jedrekjingwei.lee@digipen.edu
\par    DigiPen login: jedrekjingwei.lee

\brief
Defines the Button UI component for interactive UI elements with state-based visuals.

This header provides the Button class, which handles user interaction states
(Normal, Hovered, Pressed, Disabled) and color transitions. It includes
serialization support for saving/loading button properties.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "../Core/UITypes.h"
#include "rapidjson/document.h"

namespace Uma_UI
{
    /*!
     * \class Button
     * \brief Interactive button component with state-based color transitions.
     */
    class Button
    {
    public:
        bool interactable = true;
        Uma_UI::ButtonState currentState = Uma_UI::ButtonState::Normal;

        Uma_UI::Colour normalColour = Uma_UI::Colour::White();
        Uma_UI::Colour hoverColour = Uma_UI::Colour(0.9f, 0.9f, 0.9f, 1.0f);
        Uma_UI::Colour pressedColour = Uma_UI::Colour(0.7f, 0.7f, 0.7f, 1.0f);
        Uma_UI::Colour disabledColour = Uma_UI::Colour(0.5f, 0.5f, 0.5f, 0.5f);

        Uma_UI::UICallback onClick = nullptr;

        bool wasHoveredLastFrame = false;

        /*!
         * \brief Serializes button properties to a JSON value.
         * \param value JSON value to populate.
         * \param allocator JSON document allocator.
         */
        void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const
        {
            value.SetObject();
            value.AddMember("interactable", interactable, allocator);

            rapidjson::Value nCol(rapidjson::kObjectType);
            nCol.AddMember("r", normalColour.r, allocator);
            nCol.AddMember("g", normalColour.g, allocator);
            nCol.AddMember("b", normalColour.b, allocator);
            nCol.AddMember("a", normalColour.a, allocator);
            value.AddMember("normalColour", nCol, allocator);

            rapidjson::Value hCol(rapidjson::kObjectType);
            hCol.AddMember("r", hoverColour.r, allocator);
            hCol.AddMember("g", hoverColour.g, allocator);
            hCol.AddMember("b", hoverColour.b, allocator);
            hCol.AddMember("a", hoverColour.a, allocator);
            value.AddMember("hoverColour", hCol, allocator);

            rapidjson::Value pCol(rapidjson::kObjectType);
            pCol.AddMember("r", pressedColour.r, allocator);
            pCol.AddMember("g", pressedColour.g, allocator);
            pCol.AddMember("b", pressedColour.b, allocator);
            pCol.AddMember("a", pressedColour.a, allocator);
            value.AddMember("pressedColour", pCol, allocator);

            rapidjson::Value dCol(rapidjson::kObjectType);
            dCol.AddMember("r", disabledColour.r, allocator);
            dCol.AddMember("g", disabledColour.g, allocator);
            dCol.AddMember("b", disabledColour.b, allocator);
            dCol.AddMember("a", disabledColour.a, allocator);
            value.AddMember("disabledColour", dCol, allocator);
        }

        /*!
         * \brief Deserializes button properties from a JSON value.
         * \param value JSON value to read from.
         */
        void Deserialize(const rapidjson::Value& value)
        {
            interactable = value["interactable"].GetBool();

            const auto& nCol = value["normalColour"];
            normalColour.r = nCol["r"].GetFloat();
            normalColour.g = nCol["g"].GetFloat();
            normalColour.b = nCol["b"].GetFloat();
            normalColour.a = nCol["a"].GetFloat();

            const auto& hCol = value["hoverColour"];
            hoverColour.r = hCol["r"].GetFloat();
            hoverColour.g = hCol["g"].GetFloat();
            hoverColour.b = hCol["b"].GetFloat();
            hoverColour.a = hCol["a"].GetFloat();

            const auto& pCol = value["pressedColour"];
            pressedColour.r = pCol["r"].GetFloat();
            pressedColour.g = pCol["g"].GetFloat();
            pressedColour.b = pCol["b"].GetFloat();
            pressedColour.a = pCol["a"].GetFloat();

            const auto& dCol = value["disabledColour"];
            disabledColour.r = dCol["r"].GetFloat();
            disabledColour.g = dCol["g"].GetFloat();
            disabledColour.b = dCol["b"].GetFloat();
            disabledColour.a = dCol["a"].GetFloat();

            currentState = interactable ? Uma_UI::ButtonState::Normal : Uma_UI::ButtonState::Disabled;
            wasHoveredLastFrame = false;
            onClick = nullptr;
        }
    };
}