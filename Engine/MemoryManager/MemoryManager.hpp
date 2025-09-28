#pragma once
#include <unordered_map>
#include <cstddef>

namespace Uma_Engine
{

class MemoryManager
{
public:
    static void ReportLeaks();
    static void* Allocate(std::size_t size, const char* file, int line, const char* func);
    static void Free(void* ptr);

    static void Enable();
    static void Disable();

private:
    struct AllocInfo 
    {
        std::size_t size;
        const char* file;
        const char* func;
        int line;
    };

    static std::unordered_map<void*, AllocInfo> sMemAllocs;
    static bool sEnableTracking;
    static bool sEnableLogs;
};

} // namespace Uma_Engine


#ifdef DEBUG_MEM

inline void* operator new(std::size_t size, const char* file, int line, const char* func) 
{
    return Uma_Engine::MemoryManager::Allocate(size, file, line, func);
}

inline void* operator new[](std::size_t size, const char* file, int line, const char* func) 
{
    return Uma_Engine::MemoryManager::Allocate(size, file, line, func);
}

inline void operator delete(void* ptr) noexcept
{
    Uma_Engine::MemoryManager::Free(ptr);
}

inline void operator delete[](void* ptr) noexcept
{
    Uma_Engine::MemoryManager::Free(ptr);
}

#define __RELFILE__ (__FILE__ + sizeof(PROJECT_ROOT) - 1)
#define DEBUG_NEW new(__RELFILE__, __LINE__, __func__)
#define new DEBUG_NEW

#endif // DEBUG_MEM