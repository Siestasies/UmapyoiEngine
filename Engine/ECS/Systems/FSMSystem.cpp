/*!
\file   FSMSystem.cpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Koh Kai Yang (100%)
\par    E-mail: k.kaiyang@digipen.edu
\par    DigiPen login: k.kaiyang

\brief
System that processes state changes for FSM

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "FSMSystem.hpp"
#include "LuaScriptingSystem.hpp"

void Uma_ECS::FSMSystem::Init(Coordinator* c)
{
	pCoordinator = c;
}

void Uma_ECS::FSMSystem::Update(float dt)
{
	//add event call to lua update
	auto& FSMArray = pCoordinator->GetComponentArray<FSM>();

	for (auto const& entity : aEntities) {
		if (!pCoordinator->IsActiveInHierarchy(entity))
			continue;


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
				curr.current.pop_back();
				curr.current.pop_back();
				curr.current.pop_back();
				curr.current.pop_back();
			}

			auto state_it = curr.states.find(curr.current + ".lua");
			if (state_it != curr.states.end()) {
				auto system = pCoordinator->GetSystem<Uma_ECS::LuaScriptingSystem>();
				system->CallScriptFunction(entity, state_it->second.name, "state_enter", entity);
			}
		}
		if (!curr.current.empty()) {
			auto state_it = curr.states.find(curr.current + ".lua");
			if (state_it != curr.states.end()) {
				auto system = pCoordinator->GetSystem<Uma_ECS::LuaScriptingSystem>();
				system->CallScriptFunction(entity, state_it->second.name, "state_update", entity, dt);
			}
		}

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
}


