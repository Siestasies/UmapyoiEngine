#include "CrashLogger.hpp"
#include "Debugger.hpp"
#include <DbgHelp.h>
#pragma comment(lib, "dbghelp.lib")  // Add this line

namespace Uma_Engine
{

LONG WINAPI CrashLogger::LogCrash(EXCEPTION_POINTERS* ExceptionInfo) 
{
    // Exception Information
    DWORD code = ExceptionInfo->ExceptionRecord->ExceptionCode;
    Debugger::Log(WarningLevel::eCritical, "CRASHED: Unhandled Exception!");
    Debugger::Log(WarningLevel::eCritical, "Code: " + std::to_string(code));

    // Stack Information
    Debugger::Log(WarningLevel::eCritical, "Start of Stack Info:");

    void* stack[64]; // up to 64 frames
    USHORT frames = CaptureStackBackTrace(0, 64, stack, NULL); //skip 0 frames

    HANDLE process = GetCurrentProcess();
    SymInitialize(process, NULL, TRUE);

    // Decipher address to function names
    SYMBOL_INFO* symbol = (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
    symbol->MaxNameLen = 255;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

    for (USHORT i = 0; i < frames; i++) {
        SymFromAddr(process, (DWORD64)(stack[i]), 0, symbol);
        Debugger::Log(WarningLevel::eNoLabel,
            std::to_string(frames - i - 1) + ": " + symbol->Name +
            " - 0x" + std::to_string((uintptr_t)symbol->Address));
    }

    free(symbol);

    Debugger::Log(WarningLevel::eCritical, "End of Stack Info");

    return EXCEPTION_EXECUTE_HANDLER;
}

void CrashLogger::StartUp() 
{
    SetUnhandledExceptionFilter(CrashLogger::LogCrash);
}

} // namespace Uma_Engine