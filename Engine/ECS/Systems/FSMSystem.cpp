#include "FSMSystem.hpp"
#include "LuaScriptingSystem.hpp"

void Uma_ECS::FSMSystem::Init(Coordinator* c)
{
	pCoordinator = c;
}

void Uma_ECS::FSMSystem::Update(float dt)
{
	(void)dt;
	//add event call to lua update
	auto& FSMArray = pCoordinator->GetComponentArray<FSM>();

	for (auto const& entity : aEntities) {
		if (!pCoordinator->IsActiveInHierarchy(entity))
			continue;


		auto& curr = FSMArray.GetData(entity);
		if (curr.current.empty() && curr.next.empty()) {
			if(!curr.states.empty())
				curr.current = curr.states.begin()->first;
		}

		if (!curr.current.empty()) {
			auto system = pCoordinator->GetSystem<Uma_ECS::LuaScriptingSystem>();
			system->CallScriptFunction(entity, curr.current, "Update", dt);
		}

		if (!curr.next.empty()) {
			auto system = pCoordinator->GetSystem<Uma_ECS::LuaScriptingSystem>();
			//call exit on curr script
			system->CallScriptFunction(entity, curr.current, "onExit", entity);
			//call enter on next script
			system->CallScriptFunction(entity, curr.next, "onEnter", entity);

			//set next state to current 
			curr.current = curr.next;
			//setting next state to empty to indicate there is no more change in state
			curr.next.clear();
		}
	}
}


