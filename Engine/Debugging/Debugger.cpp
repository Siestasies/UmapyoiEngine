#include "Debugger.hpp"
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
		consoleLog = 1;
		sLogFile.open("../Logs/debug.log", std::ios::out | std::ios::trunc);
		sLogFile << std::unitbuf; // immediately write
		Log(WarningLevel::eNoLabel, "~~~~~~~~~~~~~~~~~~~~Debug~~~~~~~~~~~~~~~~~~~~~~");
	}

	void Debugger::Update(float dt)
	{
		// Crash Test
		/*int* p = nullptr;
		*p = 42;*/
	}

	void Debugger::Shutdown()
	{
		Log(WarningLevel::eNoLabel, "~~~~~~~~~~~~~~~~~~~~~End~~~~~~~~~~~~~~~~~~~~~~~");
		sLogFile.close();
	}

	void Debugger::Log(WarningLevel level, const std::string& msg)
	{
		std::lock_guard<std::mutex> lock(sLogMutex);

		std::string finalMsg;
		switch (level)
		{
		case WarningLevel::eInfo:
			finalMsg = "[Log]Info: <";
			break;
		case WarningLevel::eWarning:
			finalMsg = "[Log]Warning: <";
			break;
		case WarningLevel::eError:
			finalMsg = "[Log]Error: <";
			break;
		case WarningLevel::eCritical:
			finalMsg = "[Log]Critical: <";
			break;
		default:
			finalMsg = "<";
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

	void Debugger::Assert(bool condition, const std::string& msg)
	{

	}

} // namespace Uma_Engine