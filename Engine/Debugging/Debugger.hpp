#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <mutex>
#include "windows.h"

namespace Uma_Engine
{

    enum class WarningLevel
    {
        eInfo = 0,
        eWarning,
        eError,
        eCritical,
        eNoLabel
    };

    class Debugger
    {
    public:

        static void Init(bool console, const std::string& logfile = "../Logs/debug.log");
        static void Update();
        static void Shutdown();

        static void Log(WarningLevel level, const std::string& msg);
        static void Assert(bool terminate, const std::string& msg);

    private:

        static bool consoleLog;
        static std::ofstream sLogFile;
        static std::mutex sLogMutex;
    };

} // namespace Uma_Engine
