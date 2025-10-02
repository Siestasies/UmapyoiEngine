#pragma once
#include "SystemType.h"
#include "EventSystem.h"

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

    class Debugger : public ISystem
    {
        public:

            void Init() override;
            //void Init(bool console, const std::string& logfile = "../Logs/debug.log");
            void Update(float dt) override;
            void Shutdown() override;

            static void Log(WarningLevel level, const std::string& msg);
            static void Assert(bool condition, const std::string& msg);

        private:
            static bool consoleLog;
            static std::ofstream sLogFile;
            static std::mutex sLogMutex;

            static EventSystem* pEventSystem;
    };

} // namespace Uma_Engine
