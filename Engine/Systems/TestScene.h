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
