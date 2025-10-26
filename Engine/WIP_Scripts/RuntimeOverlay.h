#pragma once
#include "ApplicationOverlay.h"
#include "Systems/SceneManager.h"

namespace Uma_Engine
{
	class RuntimeOverlay : public ApplicationOverlay
	{
	public:
		RuntimeOverlay(SystemManager* sysm)
		{
			m_SystemManager = sysm;
		}
		~RuntimeOverlay()
		{
			m_SystemManager = nullptr;
		}

		void OnAttach() {}
		void OnDetach() {}
		void OnUpdate(float dt) {}

	private:
		// fps, drawcalls, entities
		// just debug stuff
		//SceneManager* m_SceneManager;
		SystemManager* m_SystemManager;
	};
}