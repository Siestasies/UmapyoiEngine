#pragma once

#include "../Core/UITypes.h"
#include "rapidjson/document.h"
#include "../EditorApp/Editor/Core/EditorMath.h"
#include <string>


namespace Uma_UI
{
	enum class SliderDirection
	{
		LeftToRight = 0,
		RightToLeft = 1,
		BottomToTop = 2,
		TopToBottom = 3
	};

	class Slider
	{
    public:
        // Value range
        float minValue = 0.0f;
        float maxValue = 1.0f;
        float value = 1.f;
        bool wholeNumbers = false;  // Round to integers

        // Visual settings
        SliderDirection direction = SliderDirection::LeftToRight;
        bool interactable = true;

        // Visual reference
        Uma_ECS::Entity background = static_cast<Uma_ECS::Entity>(-1);
        Uma_ECS::Entity fill = static_cast<Uma_ECS::Entity>(-1);
        Uma_ECS::Entity handle = static_cast<Uma_ECS::Entity>(-1);

        // Colors
        Uma_UI::Colour normalColour = Uma_UI::Colour::White();
        Uma_UI::Colour highlightColour = Uma_UI::Colour(0.9f, 0.9f, 0.9f, 1.0f);
        Uma_UI::Colour disabledColour = Uma_UI::Colour(0.5f, 0.5f, 0.5f, 0.5f);

        // Callback
        std::string scriptName = "";  // Script to call on value change

        // State
        bool isDragging = false;
        bool isHovered = false;

        void Serialize(rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator) const
        {
            jsonValue.SetObject();

            jsonValue.AddMember("minValue", minValue, allocator);
            jsonValue.AddMember("maxValue", maxValue, allocator);
            jsonValue.AddMember("value", value, allocator);
            jsonValue.AddMember("wholeNumbers", wholeNumbers, allocator);
            jsonValue.AddMember("direction", static_cast<int>(direction), allocator);
            jsonValue.AddMember("interactable", interactable, allocator);

            jsonValue.AddMember("backgroundEntity", static_cast<int>(background), allocator);
            jsonValue.AddMember("fillEntity", static_cast<int>(fill), allocator);
            jsonValue.AddMember("handleEntity", static_cast<int>(handle), allocator);

            jsonValue.AddMember("scriptName",
                rapidjson::Value(scriptName.c_str(), allocator), allocator);

            rapidjson::Value nCol(rapidjson::kObjectType);
            nCol.AddMember("r", normalColour.r, allocator);
            nCol.AddMember("g", normalColour.g, allocator);
            nCol.AddMember("b", normalColour.b, allocator);
            nCol.AddMember("a", normalColour.a, allocator);
            jsonValue.AddMember("normalColour", nCol, allocator);

            rapidjson::Value hCol(rapidjson::kObjectType);
            hCol.AddMember("r", highlightColour.r, allocator);
            hCol.AddMember("g", highlightColour.g, allocator);
            hCol.AddMember("b", highlightColour.b, allocator);
            hCol.AddMember("a", highlightColour.a, allocator);
            jsonValue.AddMember("highlightColour", hCol, allocator);

            rapidjson::Value dCol(rapidjson::kObjectType);
            dCol.AddMember("r", disabledColour.r, allocator);
            dCol.AddMember("g", disabledColour.g, allocator);
            dCol.AddMember("b", disabledColour.b, allocator);
            dCol.AddMember("a", disabledColour.a, allocator);
            jsonValue.AddMember("disabledColour", dCol, allocator);
        }

        void Deserialize(const rapidjson::Value& jsonValue)
        {
            minValue = jsonValue["minValue"].GetFloat();
            maxValue = jsonValue["maxValue"].GetFloat();
            value = jsonValue["value"].GetFloat();
            wholeNumbers = jsonValue["wholeNumbers"].GetBool();
            direction = static_cast<SliderDirection>(jsonValue["direction"].GetInt());
            interactable = jsonValue["interactable"].GetBool();

            if (jsonValue.HasMember("backgroundEntity"))
                background = static_cast<Uma_ECS::Entity>(jsonValue["backgroundEntity"].GetInt());
            if (jsonValue.HasMember("fillEntity"))
                fill = static_cast<Uma_ECS::Entity>(jsonValue["fillEntity"].GetInt());
            if (jsonValue.HasMember("handleEntity"))
                handle = static_cast<Uma_ECS::Entity>(jsonValue["handleEntity"].GetInt());

            if (jsonValue.HasMember("scriptName"))
            {
                scriptName = jsonValue["scriptName"].GetString();
            }

            const auto& nCol = jsonValue["normalColour"];
            normalColour.r = nCol["r"].GetFloat();
            normalColour.g = nCol["g"].GetFloat();
            normalColour.b = nCol["b"].GetFloat();
            normalColour.a = nCol["a"].GetFloat();

            const auto& hCol = jsonValue["highlightColour"];
            highlightColour.r = hCol["r"].GetFloat();
            highlightColour.g = hCol["g"].GetFloat();
            highlightColour.b = hCol["b"].GetFloat();
            highlightColour.a = hCol["a"].GetFloat();

            const auto& dCol = jsonValue["disabledColour"];
            disabledColour.r = dCol["r"].GetFloat();
            disabledColour.g = dCol["g"].GetFloat();
            disabledColour.b = dCol["b"].GetFloat();
            disabledColour.a = dCol["a"].GetFloat();

            Uma_Engine::Clamp(value, minValue, maxValue);
            isDragging = false;
            isHovered = false;
        }
	};
}