/*!
\file   FSMSystem.cpp
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A
\par    Software Engineering Project 4

\author Koh Kai Yang (100%)
\par    E-mail: k.kaiyang@digipen.edu
\par    DigiPen login: k.kaiyang

\brief
System that processes state changes for FSM

All content (C) 2026 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "FSMSystem.hpp"
#include "LuaScriptingSystem.hpp"
#include "Events/ApplicationEvents.h"
#include "AudioSystem.hpp"

void Uma_ECS::FSMSystem::Init(Coordinator* c ,Uma_Engine::EventSystem* es)
{
	pCoordinator = c;
	pEventSystem = es;
	pEventSystem->Subscribe<Uma_Engine::ApplicationGamePauseRequest, FSMSystem>(
		[this](const Uma_Engine::ApplicationGamePauseRequest& e)
		{
			GamePaused = e.pause;
			WasPaused = false;
		});
}

void Uma_ECS::FSMSystem::Update(float dt)
{
	//add event call to lua update
	auto& FSMArray = pCoordinator->GetComponentArray<FSM>();
	auto& playerArray = pCoordinator->GetComponentArray<Player>();
	auto& enemyArray = pCoordinator->GetComponentArray<Enemy>();

	for (auto const& entity : aEntities) {
		if (!pCoordinator->IsActiveInHierarchy(entity))
			continue;
		if (GamePaused) {
			if (!WasPaused) {
				if (playerArray.Has(entity) || enemyArray.Has(entity)) {
					pCoordinator->GetSystem<AudioSystem>()->StopEntitySound(entity);
				}
			}
			continue;
		}

		auto& curr = FSMArray.GetData(entity);
		if (curr.current.empty() && !curr.states.empty()) {
			// If next has a value set current to that value
			if (!curr.next.empty()) {
				curr.current = std::move(curr.next);
				curr.next.clear();
			}
			if (curr.current.empty()) {
				auto it = curr.states.begin();
				curr.current = it->first;
				//clears the first instance of ".lua" taken from file name
				curr.current.pop_back();
				curr.current.pop_back();
				curr.current.pop_back();
				curr.current.pop_back();
			}

			//adding .lua here so the user does not need to manually add .lua for every call to set next file
			auto state_it = curr.states.find(curr.current + ".lua");
			if (state_it != curr.states.end()) {
				auto system = pCoordinator->GetSystem<Uma_ECS::LuaScriptingSystem>();
				system->CallScriptFunction(entity, state_it->second.name, "state_enter", entity);
			}
		}
		//if current is not empty proceed to call update functions for the state
		if (!curr.current.empty()) {
			auto state_it = curr.states.find(curr.current + ".lua");
			if (state_it != curr.states.end()) {
				auto system = pCoordinator->GetSystem<Uma_ECS::LuaScriptingSystem>();
				system->CallScriptFunction(entity, state_it->second.name, "state_update", entity, dt);
			}
		}

		//if the next state holder is not empty begin to transition to the next state by calling exit in the current state
		//and enter for the next state then clearing the next holder
		if (!curr.next.empty() && !curr.current.empty()) {
			auto system = pCoordinator->GetSystem<Uma_ECS::LuaScriptingSystem>();

			auto curr_it = curr.states.find(curr.current + ".lua");
			if (curr_it != curr.states.end()) {
				system->CallScriptFunction(entity, curr_it->second.name, "state_exit", entity);
			}

			auto next_it = curr.states.find(curr.next + ".lua");
			if (next_it != curr.states.end()) {
				system->CallScriptFunction(entity, next_it->second.name, "state_enter", entity);
			}
			else {
				curr.next.clear();  // Prevent stuck transition
				return;
			}

			curr.current = std::move(curr.next);
			curr.next.clear();
		}

	}
	WasPaused = GamePaused;
}

void Uma_ECS::FSMSystem::Shutdown() 
{
	pEventSystem->UnsubscribeSystem<FSMSystem>();
}
