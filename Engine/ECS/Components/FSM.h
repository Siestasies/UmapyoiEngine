#pragma once
#include "Types.hpp"
#include <vector>

namespace Uma_ECS
{
	struct State
	{
		std::string name;
		bool isActive;
		int scriptIndex;
	};

	struct FSM
	{
		std::vector<state> states;
	};
}