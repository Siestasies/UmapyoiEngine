/*!
\file   Debugger.cpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Lai Jun Siang(100%)
\par    E-mail: lai.j@digipen.edu
\par    DigiPen login: lai.j

\brief
Implementation of the debugging and logging system for the game engine.
This file contains the core logging functionality including initialization of log files,
thread-safe message logging with timestamps and severity levels, optional console output,
integration with the event system for broadcasting log messages, and assertion handling
for runtime validation. The system supports multiple output targets and provides
comprehensive debugging capabilities throughout the engine.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "Debugger.hpp"
#include <ctime>
#include <vector>
#include "SystemManager.h"
#include "Events/DebugEvents.h"
#include "filepaths.h"


namespace Uma_Engine
{
	// Static member initialization
	bool			Debugger::consoleLog = false;
	std::ofstream	Debugger::sLogFile;
	std::mutex		Debugger::sLogMutex;
	EventSystem* Debugger::pEventSystem = nullptr;

	void Debugger::Init()
	{
		// Retrieve event system pointer for broadcasting log messages
		pEventSystem = pSystemManager->GetSystem<EventSystem>();

		// Disable console logging by default
		consoleLog = 1;

		// Open log file, truncating any existing content
		sLogFile.open(Uma_FilePath::LOGS_ROOT + "debug.log", std::ios::out | std::ios::trunc);
		sLogFile << std::unitbuf; // Set unbuffered mode for immediate writes

		// Log initialization marker
		Log(WarningLevel::eNoLabel, "~~~~~~~~~~~~~~~~~~~DEBUG_START~~~~~~~~~~~~~~~~~~~");
	}

	void Debugger::Update(float dt) { (void)dt; }

	void Debugger::Shutdown()
	{
		// Log shutdown marker and close the log file
		Log(WarningLevel::eNoLabel, "~~~~~~~~~~~~~~~~~~~~DEBUG_END~~~~~~~~~~~~~~~~~~~~");
		sLogFile.close();
	}

	void Debugger::Log(WarningLevel level, const std::string& msg)
	{
		// Lock mutex to ensure thread-safe logging
		std::lock_guard<std::mutex> lock(sLogMutex);

		// Get current time for timestamp
		std::time_t now = std::time(nullptr);
		std::tm localTime;
		localtime_s(&localTime, &now);

		// Format timestamp as HH:MM:SS
		char buffer[16];
		std::strftime(buffer, sizeof(buffer), "%H:%M:%S", &localTime);

		// Build log message with timestamp
		std::string finalMsg = "[" + std::string(buffer) + "] ";

		// Add severity level prefix based on warning level
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

		// Append message and closing bracket with newline
		finalMsg += msg + ">\n";

		// Write to log file if open
		if (sLogFile.is_open() && level != WarningLevel::eInfo)
			sLogFile << finalMsg;

		// Output to console if enabled
		if (consoleLog)
		{
			std::cerr << finalMsg;
		}

		// Emit log event if event system is available
		if (pEventSystem != nullptr)
		{
			pEventSystem->Emit<DebugLogEvent>(msg, level);
		}
	}

	void Debugger::Assert(bool terminate, const std::string& msg)
	{
		// If condition is false, log the assertion failure and terminate
		if (!terminate) {
			Log(WarningLevel::eCritical, "Assert Reached: " + msg);
			std::terminate();
		}
	}

	void Debugger::TestCrash()
	{
		// Crash test code - intentionally accesses out of bounds to test crash handler
		std::vector<int> nums = { 1 };
		int x = nums.at(5);
		x++;
	}

} // namespace Uma_Engine