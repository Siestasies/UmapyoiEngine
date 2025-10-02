/*!
\file   CrashLogger.hpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Lai Jun Siang(100%)
\par    E-mail: lai.j@digipen.edu
\par    DigiPen login: lai.j

\brief
Declaration of the crash logging system that captures and reports unhandled exceptions.
This header defines the CrashLogger class which provides functionality for intercepting
application crashes, capturing stack traces, and logging detailed exception information
for debugging purposes. The system integrates with Windows exception handling mechanisms
to ensure crash data is preserved before application termination.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once
#include "windows.h"

namespace Uma_Engine
{

    class CrashLogger
    {
    public:
        /*!
        * \brief Handles unhandled exceptions and logs crash information including stack trace
        * \param ExceptionInfo Pointer to Windows exception information structure
        * \return EXCEPTION_EXECUTE_HANDLER to indicate exception was handled
        */
        static LONG WINAPI LogCrash(EXCEPTION_POINTERS* ExceptionInfo);

        /*!
        * \brief Initializes the crash logging system by registering the exception handler
        */
        static void StartUp();
    };

} // namespace Uma_Engine