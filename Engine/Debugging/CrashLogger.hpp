#pragma once
#include "windows.h"

namespace Uma_Engine
{

class CrashLogger
{
public:
    static LONG WINAPI LogCrash(EXCEPTION_POINTERS* ExceptionInfo);
    static void StartUp();
};

} // namespace Uma_Engine