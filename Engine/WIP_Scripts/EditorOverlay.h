#pragma once
#include "ApplicationOverlay.h"
#include "Systems/SceneManager.h"

namespace Uma_Engine
{
	class EditorOverlay : public ApplicationOverlay
	{
	public:
		EditorOverlay(SystemManager* sysm)
		{
			m_SystemManager = sysm;
		}
		~EditorOverlay()
		{
			m_SystemManager = nullptr;
		}

		void OnAttach() {}
		void OnDetach() {}
		void OnUpdate(float dt)
		{
			RenderSceneHierarchy();
			RenderInspector();
			RenderContentBrowser();
			RenderViewport();
			RenderMenuBar();
		}

	private:
		void RenderSceneHierarchy() {}
		void RenderInspector() {}
		void RenderContentBrowser() {}
		void RenderViewport() {}
		void RenderMenuBar() {}

		//SceneManager* m_SceneManager;
		SystemManager* m_SystemManager;
		Uma_ECS::Entity m_SelectedEntity;
		bool m_ViewportFocused = false;
		bool m_ViewportHovered = false;
	};
}