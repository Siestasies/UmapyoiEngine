/*!
\file   Debugger.hpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Lai Jun Siang(100%)
\par    E-mail: lai.j@digipen.edu
\par    DigiPen login: lai.j

\brief
Declaration of the debugging and logging system for the game engine.
This header defines the Debugger class which provides centralized logging functionality
with multiple warning levels, file output capabilities, optional console output,
and integration with the event system. The system supports thread-safe logging operations
and assertion handling for runtime validation and error detection.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

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
    // Enum defining severity levels for log messages
    enum class WarningLevel
    {
        eInfo = 0,      // General information
        eWarning,       // Warning messages
        eError,         // Error conditions
        eCritical,      // Critical failures
        eNoLabel        // Raw messages without severity label
    };

    class Debugger : public ISystem
    {
    public:
        /*!
        * \brief Initializes the debugger system, opens log file and sets up event system connection
        */
        void Init() override;

        /*!
        * \brief Updates the debugger system each frame (contains crash test code)
        * \param dt Delta time since last frame
        */
        void Update(float dt) override;

        /*!
        * \brief Shuts down the debugger system and closes the log file
        */
        void Shutdown() override;

        /*!
        * \brief Logs a message with specified warning level to file, console, and event system
        * \param level Severity level of the message
        * \param msg Message string to log
        */
        static void Log(WarningLevel level, const std::string& msg);

        /*!
        * \brief Asserts a condition and terminates if false
        * \param condition Boolean condition to check
        * \param msg Error message to log if assertion fails
        */
        static void Assert(bool condition, const std::string& msg);

        /*!
        * \brief Experimental function to test crash system
        */
        static void TestCrash();

    private:
        static bool consoleLog;              // Flag to enable/disable console output
        static std::ofstream sLogFile;       // File stream for log output
        static std::mutex sLogMutex;         // Mutex for thread-safe logging

        static EventSystem* pEventSystem;    // Pointer to event system for broadcasting logs
    };

} // namespace Uma_Engine