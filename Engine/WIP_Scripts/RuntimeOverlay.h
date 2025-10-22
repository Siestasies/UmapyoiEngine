#pragma once
#include "ApplicationOverlay.h"
#include "Systems/SceneManager.h"

namespace Uma_Engine
{
	class RuntimeOverlay : public ApplicationOverlay
	{
	public:
		RuntimeOverlay(SceneManager* sm, SystemManager* sysm);
		~RuntimeOverlay();

		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate(float dt) override;
		void OnRender() override;

	private:
		// fps, drawcalls, entities
		// just debug stuff
		SceneManager* m_SceneManager;
		SystemManager* m_SystemManager;
	};
}