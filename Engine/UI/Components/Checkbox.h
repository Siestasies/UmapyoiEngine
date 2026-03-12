/*!
\file   Checkbox.h
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A
\par    Software Engineering Project 4

\author Jedrek Lee Jing Wei (100%)
\par    E-mail: jedrekjingwei.lee@digipen.edu
\par    DigiPen login: jedrekjingwei.lee

\brief
Defines the Checkbox UI component for toggleable checkboxes with visual state
feedback including normal, hover, pressed, disabled, and checked colour states.
Supports JSON serialization and deserialization of all visual properties.

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
     * \class Checkbox
     * \brief UI component for a toggleable checkbox with visual state feedback.
     */
    class Checkbox
    {
    public:
        // State
        bool isChecked = true;
        bool interactable = true;

        // Visual settings
        CheckboxState currentState = CheckboxState::Normal;

        // Visual reference
        Uma_ECS::Entity background = static_cast<Uma_ECS::Entity>(-1);
        Uma_ECS::Entity checkmark = static_cast<Uma_ECS::Entity>(-1);

        // Colours
        Uma_UI::Color normalColour = Uma_UI::Color::White();
        Uma_UI::Color hoverColour = Uma_UI::Color(0.9f, 0.9f, 0.9f, 1.0f);
        Uma_UI::Color pressedColour = Uma_UI::Color(0.7f, 0.7f, 0.7f, 1.0f);
        Uma_UI::Color disabledColour = Uma_UI::Color(0.5f, 0.5f, 0.5f, 0.5f);
        Uma_UI::Color checkedColour = Uma_UI::Color(0.2f, 0.6f, 1.0f, 1.0f);

        // Checkmark colours
        Uma_UI::Color checkmarkNormalColour = Uma_UI::Color::Black();
        Uma_UI::Color checkmarkDisabledColour = Uma_UI::Color::Gray();

        // Callback
        std::string scriptName = "";  // Script to call on toggle

        // Runtime state
        bool wasHoveredLastFrame = false;
        bool wasPressedWhileHovered = false;

        /*!
         * \brief Serializes checkbox settings to a JSON value.
         * \param jsonValue Output JSON value to populate.
         * \param allocator RapidJSON allocator for memory management.
         */
        void Serialize(rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator) const
        {
            jsonValue.SetObject();

            jsonValue.AddMember("isChecked", isChecked, allocator);
            jsonValue.AddMember("interactable", interactable, allocator);

            jsonValue.AddMember("backgroundEntity", static_cast<int>(background), allocator);
            jsonValue.AddMember("checkmarkEntity", static_cast<int>(checkmark), allocator);

            jsonValue.AddMember("scriptName",
                rapidjson::Value(scriptName.c_str(), allocator), allocator);

            rapidjson::Value nCol(rapidjson::kObjectType);
            nCol.AddMember("r", normalColour.r, allocator);
            nCol.AddMember("g", normalColour.g, allocator);
            nCol.AddMember("b", normalColour.b, allocator);
            nCol.AddMember("a", normalColour.a, allocator);
            jsonValue.AddMember("normalColour", nCol, allocator);

            rapidjson::Value hCol(rapidjson::kObjectType);
            hCol.AddMember("r", hoverColour.r, allocator);
            hCol.AddMember("g", hoverColour.g, allocator);
            hCol.AddMember("b", hoverColour.b, allocator);
            hCol.AddMember("a", hoverColour.a, allocator);
            jsonValue.AddMember("hoverColour", hCol, allocator);

            rapidjson::Value pCol(rapidjson::kObjectType);
            pCol.AddMember("r", pressedColour.r, allocator);
            pCol.AddMember("g", pressedColour.g, allocator);
            pCol.AddMember("b", pressedColour.b, allocator);
            pCol.AddMember("a", pressedColour.a, allocator);
            jsonValue.AddMember("pressedColour", pCol, allocator);

            rapidjson::Value dCol(rapidjson::kObjectType);
            dCol.AddMember("r", disabledColour.r, allocator);
            dCol.AddMember("g", disabledColour.g, allocator);
            dCol.AddMember("b", disabledColour.b, allocator);
            dCol.AddMember("a", disabledColour.a, allocator);
            jsonValue.AddMember("disabledColour", dCol, allocator);

            rapidjson::Value cCol(rapidjson::kObjectType);
            cCol.AddMember("r", checkedColour.r, allocator);
            cCol.AddMember("g", checkedColour.g, allocator);
            cCol.AddMember("b", checkedColour.b, allocator);
            cCol.AddMember("a", checkedColour.a, allocator);
            jsonValue.AddMember("checkedColour", cCol, allocator);

            rapidjson::Value cmCol(rapidjson::kObjectType);
            cmCol.AddMember("r", checkmarkNormalColour.r, allocator);
            cmCol.AddMember("g", checkmarkNormalColour.g, allocator);
            cmCol.AddMember("b", checkmarkNormalColour.b, allocator);
            cmCol.AddMember("a", checkmarkNormalColour.a, allocator);
            jsonValue.AddMember("checkmarkNormalColour", cmCol, allocator);

            rapidjson::Value cmdCol(rapidjson::kObjectType);
            cmdCol.AddMember("r", checkmarkDisabledColour.r, allocator);
            cmdCol.AddMember("g", checkmarkDisabledColour.g, allocator);
            cmdCol.AddMember("b", checkmarkDisabledColour.b, allocator);
            cmdCol.AddMember("a", checkmarkDisabledColour.a, allocator);
            jsonValue.AddMember("checkmarkDisabledColour", cmdCol, allocator);
        }

        /*!
         * \brief Deserializes checkbox settings from a JSON value.
         * \param jsonValue Input JSON value to read from.
         */
        void Deserialize(const rapidjson::Value& jsonValue)
        {
            isChecked = jsonValue["isChecked"].GetBool();
            interactable = jsonValue["interactable"].GetBool();

            if (jsonValue.HasMember("backgroundEntity"))
                background = static_cast<Uma_ECS::Entity>(jsonValue["backgroundEntity"].GetInt());
            if (jsonValue.HasMember("checkmarkEntity"))
                checkmark = static_cast<Uma_ECS::Entity>(jsonValue["checkmarkEntity"].GetInt());

            if (jsonValue.HasMember("scriptName"))
            {
                scriptName = jsonValue["scriptName"].GetString();
            }

            const auto& nCol = jsonValue["normalColour"];
            normalColour.r = nCol["r"].GetFloat();
            normalColour.g = nCol["g"].GetFloat();
            normalColour.b = nCol["b"].GetFloat();
            normalColour.a = nCol["a"].GetFloat();

            const auto& hCol = jsonValue["hoverColour"];
            hoverColour.r = hCol["r"].GetFloat();
            hoverColour.g = hCol["g"].GetFloat();
            hoverColour.b = hCol["b"].GetFloat();
            hoverColour.a = hCol["a"].GetFloat();

            const auto& pCol = jsonValue["pressedColour"];
            pressedColour.r = pCol["r"].GetFloat();
            pressedColour.g = pCol["g"].GetFloat();
            pressedColour.b = pCol["b"].GetFloat();
            pressedColour.a = pCol["a"].GetFloat();

            const auto& dCol = jsonValue["disabledColour"];
            disabledColour.r = dCol["r"].GetFloat();
            disabledColour.g = dCol["g"].GetFloat();
            disabledColour.b = dCol["b"].GetFloat();
            disabledColour.a = dCol["a"].GetFloat();

            const auto& cCol = jsonValue["checkedColour"];
            checkedColour.r = cCol["r"].GetFloat();
            checkedColour.g = cCol["g"].GetFloat();
            checkedColour.b = cCol["b"].GetFloat();
            checkedColour.a = cCol["a"].GetFloat();

            const auto& cmCol = jsonValue["checkmarkNormalColour"];
            checkmarkNormalColour.r = cmCol["r"].GetFloat();
            checkmarkNormalColour.g = cmCol["g"].GetFloat();
            checkmarkNormalColour.b = cmCol["b"].GetFloat();
            checkmarkNormalColour.a = cmCol["a"].GetFloat();

            const auto& cmdCol = jsonValue["checkmarkDisabledColour"];
            checkmarkDisabledColour.r = cmdCol["r"].GetFloat();
            checkmarkDisabledColour.g = cmdCol["g"].GetFloat();
            checkmarkDisabledColour.b = cmdCol["b"].GetFloat();
            checkmarkDisabledColour.a = cmdCol["a"].GetFloat();

            currentState = interactable ? CheckboxState::Normal : CheckboxState::Disabled;
            wasHoveredLastFrame = false;
        }
    };
}