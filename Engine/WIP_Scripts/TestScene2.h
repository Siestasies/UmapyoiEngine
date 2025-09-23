#pragma once
#include "SceneType.h"
#include <iostream>

namespace Uma_Engine
{
	class TestScene2 : public Scene
	{
		private:
			SystemManager* pSystemManagerRef;

		public:
			TestScene2(SystemManager* sm) : pSystemManagerRef(sm) {}
		void OnLoad() override
		{
			// parse files here
			std::cout << "Test Scene 2: LOADED" << std::endl;
		}
		void OnUnload() override
		{
			std::cout << "Test Scene 2: UN LOADED" << std::endl;

		}
		void Update(float dt) override
		{

		}
		void Render() override
		{

		}
	};
}
