#pragma once
#include "ApplicationOverlay.h"
#include "RuntimeOverlay.h"
#include "EditorOverlay.h"
#include "Core/SystemType.h"
#include "Systems/SceneManager.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <iostream>

namespace Uma_Engine
{
	class OverlayManager : public ISystem
	{
	public:
		OverlayManager() = default;

		// Optional: allow injection later
		//void SetSceneManager(SceneManager* sm) { m_SceneManager = sm; }

		// --- ISystem interface ---
		void Init() override
		{
			//if (!m_SceneManager)
			//{
			//	std::cerr << "[OverlayManager] SceneManager not set before Init().\n";
			//	return;
			//}

			auto sysManager = pSystemManager; // inherited from ISystem

			m_Overlays["Editor"] = std::make_shared<EditorOverlay>(/*m_SceneManager, */sysManager);
			m_Overlays["Runtime"] = std::make_shared<RuntimeOverlay>(/*m_SceneManager, */sysManager);

			for (auto& [name, overlay] : m_Overlays)
				overlay->OnAttach();

			m_ActiveOverlay = m_Overlays["Editor"];
		}

		void Update(float dt) override
		{
			if (m_ActiveOverlay)
				m_ActiveOverlay->OnUpdate(dt);
		}

		void Shutdown() override
		{
			for (auto& [name, overlay] : m_Overlays)
				overlay->OnDetach();
		}
		// -------------------------

		void SetActiveOverlay(const std::string& name)
		{
			auto it = m_Overlays.find(name);
			if (it != m_Overlays.end())
				m_ActiveOverlay = it->second;
			else
				std::cerr << "[OverlayManager] Overlay '" << name << "' not found.\n";
		}

	private:
		//SceneManager* m_SceneManager = nullptr;
		std::unordered_map<std::string, std::shared_ptr<ApplicationOverlay>> m_Overlays;
		std::shared_ptr<ApplicationOverlay> m_ActiveOverlay = nullptr;
	};
}
