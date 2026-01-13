#pragma once
#include "../Engine/ECS/Core/Types.hpp"
#include "unordered_map"

namespace Uma_ECS
{
	struct State
	{
		std::string name;
		bool isActive;
		//int scriptIndex;
	};

	struct FSM
	{
		//stores all states
		//std::vector<State> states;

		std::unordered_map<std::string, State> states;

		std::string current;
		std::string next;

		bool ChangeStates(const std::string& name) {
			auto it = states.find(name);
			if (it == states.end() || name.empty()) {
				//if state not found return
				//can log error if needed
				return false;
			}

			//to allow for exit/enter code to run before the transition
			next = name;
			return true;
		};

		//bool to indicate successful adding of state
		bool AddStates(const std::string& name, bool isActive) {
			auto it = states.find(name);
			if (it != states.end() || name.empty()) {
				//if it exists return to stop overriding of the state
				return false;
			}

			states[name] = State(name, isActive);
			return true;
		}
	};
}