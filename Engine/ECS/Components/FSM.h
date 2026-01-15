#pragma once
#include "../Engine/ECS/Core/Types.hpp"
#include "unordered_map"

namespace Uma_ECS
{
	struct State
	{
		std::string name;
		bool isActive = true;
		int scriptIndex = -1;
	};

	struct FSM
	{
		//stores all states
		//std::vector<State> states;

		std::unordered_map<std::string, State> states;

		unsigned int nextId = 0;

		std::string current;
		std::string next;

		//expose this to lua
		bool ChangeStates(const std::string& name) {
			if (name.empty()) return false;
			auto it = states.find(name);
			//if name not found or if the script is inactive dont change to the state
			if (it == states.end() || !it->second.isActive) {
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
			if (name.empty()) return false;

			auto [iter, inserted] = states.emplace(name, State{ name, isActive, static_cast<int>(nextId) });
			if (!inserted) {
				// already exists, do not override
				return false;
			}
			++nextId;
			return true;
		}

		void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const //override
		{
			(void)value;
			(void)allocator;
		}

		// Deserialize from JSON
		void Deserialize(const rapidjson::Value& value) //override
		{
			(void)value;
		}
	};

}