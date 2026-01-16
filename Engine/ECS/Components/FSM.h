#pragma once
#include "../Core/Types.hpp"
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
		std::vector<State> states;
	};
}