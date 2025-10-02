#include "Debugger.hpp"
#include <ctime>
#include <vector>
#include "SystemManager.h"
#include "DebugEvents.h"

namespace Uma_Engine
{
	bool			Debugger::consoleLog = false;
	std::ofstream	Debugger::sLogFile;
	std::mutex		Debugger::sLogMutex;
	EventSystem* Debugger::pEventSystem = nullptr;

	void Debugger::Init()
	{
		pEventSystem = pSystemManager->GetSystem<EventSystem>();
		consoleLog = 0;
		sLogFile.open("../Logs/debug.log", std::ios::out | std::ios::trunc);
		sLogFile << std::unitbuf; // immediately write
		Log(WarningLevel::eNoLabel, "~~~~~~~~~~~~~~~~~~~DEBUG_START~~~~~~~~~~~~~~~~~~~");
	}

	void Debugger::Update(float dt)
	{
		// Crash Test
		std::vector<int> nums = { 1 };
		int x = nums.at(5);
	}

	void Debugger::Shutdown()
	{
		Log(WarningLevel::eNoLabel, "~~~~~~~~~~~~~~~~~~~~DEBUG_END~~~~~~~~~~~~~~~~~~~~");
		sLogFile.close();
	}

	void Debugger::Log(WarningLevel level, const std::string& msg)
	{
		std::lock_guard<std::mutex> lock(sLogMutex);

		std::time_t now = std::time(nullptr);
		std::tm localTime;
		localtime_s(&localTime, &now);

		char buffer[16];
		std::strftime(buffer, sizeof(buffer), "%H:%M:%S", &localTime);

		std::string finalMsg = "[" + std::string(buffer) + "] ";
		switch (level)
		{
		case WarningLevel::eInfo:
			finalMsg += "[Log]Info: <";
			break;
		case WarningLevel::eWarning:
			finalMsg += "[Log]Warning: <";
			break;
		case WarningLevel::eError:
			finalMsg += "[Log]Error: <";
			break;
		case WarningLevel::eCritical:
			finalMsg += "[Log]Critical: <";
			break;
		default:
			finalMsg += "<";
			break;
		}

		finalMsg += msg + ">\n";
		if (sLogFile.is_open())
			sLogFile << finalMsg;

		if (consoleLog)
		{
			std::cerr << finalMsg;
		}

		if (pEventSystem != nullptr)
		{
			pEventSystem->Emit<DebugLogEvent>(msg, level);
		}
	}

	void Debugger::Assert(bool terminate, const std::string& msg)
	{
		if (!terminate) {
			Log(WarningLevel::eCritical, "Assert Reached: " + msg);
			std::terminate();
		}
	}

} // namespace Uma_Engine