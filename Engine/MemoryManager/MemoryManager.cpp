#include "MemoryManager.hpp"
#include "../Debugging/Debugger.hpp"
#include <cstdlib>
#include <new>

namespace Uma_Engine
{

bool MemoryManager::sEnableLogs = false;
bool MemoryManager::sEnableTracking = false;
std::unordered_map<void*, MemoryManager::AllocInfo> MemoryManager::sMemAllocs;

void* MemoryManager::Allocate(std::size_t size, const char* file, int line, const char* func) 
{
    void* ptr = std::malloc(size);
    if (!ptr)
    {
        Debugger::Log(WarningLevel::eError,
            "Bad Alloc of " + std::to_string(size) + " bytes at " + file +
            ", line " + std::to_string(line) + " in \"" + func + "\"");

        throw std::bad_alloc();
    }

    if (!sEnableTracking)
        return ptr;

    sMemAllocs[ptr] = { size, file, func, line };

    if (sEnableLogs)
        Debugger::Log(WarningLevel::eInfo,
            "Allocated " + std::to_string(size) + " bytes at " + file + 
            ", line " + std::to_string(line) + " in \"" + func + "\"");

    return ptr;
}

void MemoryManager::Free(void* ptr) 
{
    if (!sEnableTracking)
    {
        std::free(ptr);
        return;
    }

    std::unordered_map<void*, MemoryManager::AllocInfo>::const_iterator it;
    if ( ( it = sMemAllocs.find(ptr) ) != sMemAllocs.end() ) 
    {
        if (sEnableLogs)
            Debugger::Log(WarningLevel::eInfo,
                "Freed " + std::to_string(it->second.size) + " bytes from " + it->second.file +
                ", line " + std::to_string(it->second.line) + " in \"" + it->second.func + "\"");

        sMemAllocs.erase(it);
    }
    std::free(ptr);
}

void MemoryManager::ReportLeaks() 
{
    if (!sMemAllocs.empty()) 
    {
        Debugger::Log(WarningLevel::eError, "=== Memory leaks Detected ===");
        for (auto& [ptr, info] : sMemAllocs) 
        {
            Debugger::Log(WarningLevel::eError,
                "Leak of " + std::to_string(info.size) + " bytes from " + info.file +
                ", line " + std::to_string(info.line) + " in \"" + info.func + "\"");
        }
    }
    else 
    {
        Debugger::Log(WarningLevel::eInfo, "=== No Memory leaks Detected ===");
    }
}

void MemoryManager::Enable()
{
    sEnableTracking = true;
}

void MemoryManager::Disable()
{
    sEnableTracking = false;
}

} // namespace Uma_Engine