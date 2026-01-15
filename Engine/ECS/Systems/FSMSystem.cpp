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

		if (!curr.next.empty()) {
			//call exit on curr script
			//call enter on next script

			//set next state to current 
			curr.current = curr.next;
			//setting next state to empty to indicate there is no more change in state
			curr.next.clear();
		}
	}
}


