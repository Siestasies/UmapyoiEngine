#pragma once

#include "../Core/UITypes.h"
#include "rapidjson/document.h"
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
        float value = 0.5f;
        bool wholeNumbers = false;  // Round to integers

        // Visual settings
        SliderDirection direction = SliderDirection::LeftToRight;
        bool interactable = true;

        // Colors
        Uma_UI::Color normalColor = Uma_UI::Color::White();
        Uma_UI::Color highlightColor = Uma_UI::Color(0.9f, 0.9f, 0.9f, 1.0f);
        Uma_UI::Color disabledColor = Uma_UI::Color(0.5f, 0.5f, 0.5f, 0.5f);

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

            jsonValue.AddMember("scriptName",
                rapidjson::Value(scriptName.c_str(), allocator), allocator);

            rapidjson::Value nCol(rapidjson::kObjectType);
            nCol.AddMember("r", normalColor.r, allocator);
            nCol.AddMember("g", normalColor.g, allocator);
            nCol.AddMember("b", normalColor.b, allocator);
            nCol.AddMember("a", normalColor.a, allocator);
            jsonValue.AddMember("normalColor", nCol, allocator);

            rapidjson::Value hCol(rapidjson::kObjectType);
            hCol.AddMember("r", highlightColor.r, allocator);
            hCol.AddMember("g", highlightColor.g, allocator);
            hCol.AddMember("b", highlightColor.b, allocator);
            hCol.AddMember("a", highlightColor.a, allocator);
            jsonValue.AddMember("highlightColor", hCol, allocator);

            rapidjson::Value dCol(rapidjson::kObjectType);
            dCol.AddMember("r", disabledColor.r, allocator);
            dCol.AddMember("g", disabledColor.g, allocator);
            dCol.AddMember("b", disabledColor.b, allocator);
            dCol.AddMember("a", disabledColor.a, allocator);
            jsonValue.AddMember("disabledColor", dCol, allocator);
        }

        void Deserialize(const rapidjson::Value& jsonValue)
        {
            minValue = jsonValue["minValue"].GetFloat();
            maxValue = jsonValue["maxValue"].GetFloat();
            value = jsonValue["value"].GetFloat();
            wholeNumbers = jsonValue["wholeNumbers"].GetBool();
            direction = static_cast<SliderDirection>(jsonValue["direction"].GetInt());
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

            const auto& hCol = jsonValue["highlightColor"];
            highlightColor.r = hCol["r"].GetFloat();
            highlightColor.g = hCol["g"].GetFloat();
            highlightColor.b = hCol["b"].GetFloat();
            highlightColor.a = hCol["a"].GetFloat();

            const auto& dCol = jsonValue["disabledColor"];
            disabledColor.r = dCol["r"].GetFloat();
            disabledColor.g = dCol["g"].GetFloat();
            disabledColor.b = dCol["b"].GetFloat();
            disabledColor.a = dCol["a"].GetFloat();

            ClampValue();
            isDragging = false;
            isHovered = false;
        }
	};
}