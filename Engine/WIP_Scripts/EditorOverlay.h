#pragma once
#include "ApplicationOverlay.h"
#include "Systems/SceneManager.h"

namespace Uma_Engine
{
	class EditorOverlay : public ApplicationOverlay
	{
	public:
		EditorOverlay(SceneManager* sm, SystemManager* sysm);
		~EditorOverlay();

		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate(float dt) override;
		void OnRender() override;

	private:
		void RenderSceneHierarchy();
		void RenderInspector();
		void RenderContentBrowser();
		void RenderViewport();
		void RenderMenuBar();

		SceneManager* m_SceneManager;
		SystemManager* m_SystemManager;
		Uma_ECS::Entity m_SelectedEntity;
		bool m_ViewportFocused = false;
		bool m_ViewportHovered = false;
	};
}