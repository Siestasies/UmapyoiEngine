#pragma once

#include "../Core/UITypes.h"
#include "rapidjson/document.h"
#include <string>

namespace Uma_UI
{
    enum class CheckboxState
    {
        Normal = 0,
        Hovered = 1,
        Pressed = 2,
        Disabled = 3
    };

    class Checkbox
    {
    public:
        // State
        bool isChecked = false;
        bool interactable = true;

        // Visual settings
        CheckboxState currentState = CheckboxState::Normal;

        // Colors
        Uma_UI::Color normalColor = Uma_UI::Color::White();
        Uma_UI::Color hoverColor = Uma_UI::Color(0.9f, 0.9f, 0.9f, 1.0f);
        Uma_UI::Color pressedColor = Uma_UI::Color(0.7f, 0.7f, 0.7f, 1.0f);
        Uma_UI::Color disabledColor = Uma_UI::Color(0.5f, 0.5f, 0.5f, 0.5f);
        Uma_UI::Color checkedColor = Uma_UI::Color(0.2f, 0.6f, 1.0f, 1.0f);

        // Checkmark colors
        Uma_UI::Color checkmarkNormalColor = Uma_UI::Color::Black();
        Uma_UI::Color checkmarkDisabledColor = Uma_UI::Color::Gray();

        // Callback
        std::string scriptName = "";  // Script to call on toggle

        // Runtime state
        bool wasHoveredLastFrame = false;

        void Serialize(rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator) const
        {
            jsonValue.SetObject();

            jsonValue.AddMember("isChecked", isChecked, allocator);
            jsonValue.AddMember("interactable", interactable, allocator);

            jsonValue.AddMember("scriptName",
                rapidjson::Value(scriptName.c_str(), allocator), allocator);

            rapidjson::Value nCol(rapidjson::kObjectType);
            nCol.AddMember("r", normalColor.r, allocator);
            nCol.AddMember("g", normalColor.g, allocator);
            nCol.AddMember("b", normalColor.b, allocator);
            nCol.AddMember("a", normalColor.a, allocator);
            jsonValue.AddMember("normalColor", nCol, allocator);

            rapidjson::Value hCol(rapidjson::kObjectType);
            hCol.AddMember("r", hoverColor.r, allocator);
            hCol.AddMember("g", hoverColor.g, allocator);
            hCol.AddMember("b", hoverColor.b, allocator);
            hCol.AddMember("a", hoverColor.a, allocator);
            jsonValue.AddMember("hoverColor", hCol, allocator);

            rapidjson::Value pCol(rapidjson::kObjectType);
            pCol.AddMember("r", pressedColor.r, allocator);
            pCol.AddMember("g", pressedColor.g, allocator);
            pCol.AddMember("b", pressedColor.b, allocator);
            pCol.AddMember("a", pressedColor.a, allocator);
            jsonValue.AddMember("pressedColor", pCol, allocator);

            rapidjson::Value dCol(rapidjson::kObjectType);
            dCol.AddMember("r", disabledColor.r, allocator);
            dCol.AddMember("g", disabledColor.g, allocator);
            dCol.AddMember("b", disabledColor.b, allocator);
            dCol.AddMember("a", disabledColor.a, allocator);
            jsonValue.AddMember("disabledColor", dCol, allocator);

            rapidjson::Value cCol(rapidjson::kObjectType);
            cCol.AddMember("r", checkedColor.r, allocator);
            cCol.AddMember("g", checkedColor.g, allocator);
            cCol.AddMember("b", checkedColor.b, allocator);
            cCol.AddMember("a", checkedColor.a, allocator);
            jsonValue.AddMember("checkedColor", cCol, allocator);

            rapidjson::Value cmCol(rapidjson::kObjectType);
            cmCol.AddMember("r", checkmarkNormalColor.r, allocator);
            cmCol.AddMember("g", checkmarkNormalColor.g, allocator);
            cmCol.AddMember("b", checkmarkNormalColor.b, allocator);
            cmCol.AddMember("a", checkmarkNormalColor.a, allocator);
            jsonValue.AddMember("checkmarkNormalColor", cmCol, allocator);

            rapidjson::Value cmdCol(rapidjson::kObjectType);
            cmdCol.AddMember("r", checkmarkDisabledColor.r, allocator);
            cmdCol.AddMember("g", checkmarkDisabledColor.g, allocator);
            cmdCol.AddMember("b", checkmarkDisabledColor.b, allocator);
            cmdCol.AddMember("a", checkmarkDisabledColor.a, allocator);
            jsonValue.AddMember("checkmarkDisabledColor", cmdCol, allocator);
        }

        void Deserialize(const rapidjson::Value& jsonValue)
        {
            isChecked = jsonValue["isChecked"].GetBool();
            interactable = jsonValue["interactable"].GetBool();

            if (jsonValue.HasMember("scriptName"))
            {
                scriptName = jsonValue["scriptName"].GetString();
            }

            const auto& nCol = jsonValue["normalColor"];
            normalColor.r = nCol["r"].GetFloat();
            normalColor.g = nCol["g"].GetFloat();
            normalColor.b = nCol["b"].GetFloat();
            normalColor.a = nCol["a"].GetFloat();

            const auto& hCol = jsonValue["hoverColor"];
            hoverColor.r = hCol["r"].GetFloat();
            hoverColor.g = hCol["g"].GetFloat();
            hoverColor.b = hCol["b"].GetFloat();
            hoverColor.a = hCol["a"].GetFloat();

            const auto& pCol = jsonValue["pressedColor"];
            pressedColor.r = pCol["r"].GetFloat();
            pressedColor.g = pCol["g"].GetFloat();
            pressedColor.b = pCol["b"].GetFloat();
            pressedColor.a = pCol["a"].GetFloat();

            const auto& dCol = jsonValue["disabledColor"];
            disabledColor.r = dCol["r"].GetFloat();
            disabledColor.g = dCol["g"].GetFloat();
            disabledColor.b = dCol["b"].GetFloat();
            disabledColor.a = dCol["a"].GetFloat();

            const auto& cCol = jsonValue["checkedColor"];
            checkedColor.r = cCol["r"].GetFloat();
            checkedColor.g = cCol["g"].GetFloat();
            checkedColor.b = cCol["b"].GetFloat();
            checkedColor.a = cCol["a"].GetFloat();

            const auto& cmCol = jsonValue["checkmarkNormalColor"];
            checkmarkNormalColor.r = cmCol["r"].GetFloat();
            checkmarkNormalColor.g = cmCol["g"].GetFloat();
            checkmarkNormalColor.b = cmCol["b"].GetFloat();
            checkmarkNormalColor.a = cmCol["a"].GetFloat();

            const auto& cmdCol = jsonValue["checkmarkDisabledColor"];
            checkmarkDisabledColor.r = cmdCol["r"].GetFloat();
            checkmarkDisabledColor.g = cmdCol["g"].GetFloat();
            checkmarkDisabledColor.b = cmdCol["b"].GetFloat();
            checkmarkDisabledColor.a = cmdCol["a"].GetFloat();

            currentState = interactable ? CheckboxState::Normal : CheckboxState::Disabled;
            wasHoveredLastFrame = false;
        }
    };
}