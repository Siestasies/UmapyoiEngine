#pragma once
#include "SceneType.h"

#include <iostream>

namespace Uma_Engine
{
	class TestScene : public Scene
	{
		void OnLoad() override
		{
			// parse files here
			std::cout << "Test Scene 1: LOADED" << std::endl;
			const char* json = R"({
							"hello":"world",
							"t":true,
							"f":false,
							"n":null,
							"i":123,
							"pi":3.1416
						})";
			rapidjson::Document doc;
			doc.Parse(json);

			if (doc.HasMember("hello"))
			{
				std::cout << "RAPIDJSON WORKING: " << doc["pi"].GetDouble() << std::endl;
			}

		}
		void OnUnload() override
		{
			std::cout << "Test Scene 1: UNloaded" << std::endl;

		}
		void Update(float dt) override
		{

		}
		void Render() override
		{

		}
	};
}
