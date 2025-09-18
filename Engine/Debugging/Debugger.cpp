#include "Debugger.hpp"
namespace Uma_Engine
{
	bool			Debugger::consoleLog = false;
	std::ofstream	Debugger::sLogFile;
	std::mutex		Debugger::sLogMutex;

	void Debugger::Init(bool console, const std::string& filename)
	{
		consoleLog = console;
		sLogFile.open(filename, std::ios::out | std::ios::trunc);
		sLogFile << std::unitbuf; // immediately write
		Log(WarningLevel::eNoLabel, "~~~~~~~~~~~~~~~~~~~~Debug~~~~~~~~~~~~~~~~~~~~~~");
	}

	void Debugger::Update()
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
			std::cerr << finalMsg;
	}

	void Debugger::Assert(bool condition, const std::string& msg)
	{

	}


} // namespace Uma_Engine