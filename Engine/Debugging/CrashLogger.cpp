/*!
\file   CrashLogger.cpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Lai Jun Siang(100%)
\par    E-mail: lai.j@digipen.edu
\par    DigiPen login: lai.j

\brief
Implementation of the crash logging system that captures and reports unhandled exceptions.
This file contains the core functionality for intercepting application crashes using Windows
exception handling, capturing detailed stack traces using DbgHelp library, resolving function
addresses to symbolic names, and logging all crash information through the Debugger system.
The implementation ensures critical debugging information is preserved when the application
encounters fatal errors.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "CrashLogger.hpp"
#include "Debugger.hpp"
#include <DbgHelp.h>
#pragma comment(lib, "dbghelp.lib")  // Link against debug help library for stack trace functionality

namespace Uma_Engine
{

    LONG WINAPI CrashLogger::LogCrash(EXCEPTION_POINTERS* ExceptionInfo)
    {
        // Extract exception code from the exception record
        DWORD code = ExceptionInfo->ExceptionRecord->ExceptionCode;
        Debugger::Log(WarningLevel::eCritical, "CRASHED: Unhandled Exception!");
        Debugger::Log(WarningLevel::eCritical, "Code: " + std::to_string(code));

        // Begin stack trace capture
        Debugger::Log(WarningLevel::eCritical, "Start of Stack Info:");

        // Capture stack frames (up to 64 levels deep)
        void* stack[64];
        USHORT frames = CaptureStackBackTrace(0, 64, stack, NULL); // Skip 0 frames to include all

        // Initialize symbol handler for the current process
        HANDLE process = GetCurrentProcess();
        SymInitialize(process, NULL, TRUE);

        // Allocate symbol information structure with space for function name
        SYMBOL_INFO* symbol = (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
        symbol->MaxNameLen = 255;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

        // Iterate through captured stack frames and resolve addresses to function names
        for (USHORT i = 0; i < frames; i++) {
            SymFromAddr(process, (DWORD64)(stack[i]), 0, symbol);
            // Log frame number (in reverse order), function name, and address
            Debugger::Log(WarningLevel::eNoLabel,
                std::to_string(frames - i - 1) + ": " + symbol->Name +
                " - 0x" + std::to_string((uintptr_t)symbol->Address));
        }

        // Clean up allocated symbol information
        free(symbol);

        Debugger::Log(WarningLevel::eCritical, "End of Stack Info");

        // Indicate that the exception has been handled
        return EXCEPTION_EXECUTE_HANDLER;
    }

    void CrashLogger::StartUp()
    {
        // Register the crash handler to intercept unhandled exceptions
        SetUnhandledExceptionFilter(CrashLogger::LogCrash);
    }

} // namespace Uma_Engine