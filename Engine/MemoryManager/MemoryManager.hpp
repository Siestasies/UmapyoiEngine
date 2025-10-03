/*!
\file   MemoryManager.hpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3
\author Lai Jun Siang(100%)
\par    E-mail: lai.j@digipen.edu
\par    DigiPen login: lai.j
\brief
Declaration of the memory management and tracking system for debugging memory leaks.
This header defines the MemoryManager class which provides custom memory allocation
and deallocation with tracking capabilities. The system records allocation metadata
including file location, line number, and function name to enable comprehensive
memory leak detection and reporting. When DEBUG_MEM is defined, it overrides the
global new/delete operators to automatically track all dynamic memory allocations.
All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once
#include <unordered_map>
#include <cstddef>

namespace Uma_Engine
{

    class MemoryManager
    {
    public:
        /*!
        * \brief Reports all detected memory leaks with allocation details
        */
        static void ReportLeaks();

        /*!
        * \brief Allocates memory and tracks allocation metadata for leak detection
        * \param size Number of bytes to allocate
        * \param file Source file where allocation occurred
        * \param line Line number where allocation occurred
        * \param func Function name where allocation occurred
        * \return Pointer to allocated memory
        */
        static void* Allocate(std::size_t size, const char* file, int line, const char* func);

        /*!
        * \brief Frees previously allocated memory and removes from tracking
        * \param ptr Pointer to memory to deallocate
        */
        static void Free(void* ptr);

        /*!
        * \brief Enables memory tracking for leak detection
        */
        static void Enable();

        /*!
        * \brief Disables memory tracking for leak detection
        */
        static void Disable();

    private:
        // Structure to store metadata about each allocation
        struct AllocInfo
        {
            std::size_t size;   // Size of allocation in bytes
            const char* file;   // Source file name
            const char* func;   // Function name
            int line;           // Line number
        };

        static std::unordered_map<void*, AllocInfo> sMemAllocs;  // Map of active allocations
        static bool sEnableTracking;                              // Flag to enable/disable tracking
        static bool sEnableLogs;                                  // Flag to enable/disable logging
    };

} // namespace Uma_Engine


#ifdef DEBUG_MEM

// Override global new operator to track allocations
inline void* operator new(std::size_t size, const char* file, int line, const char* func)
{
    return Uma_Engine::MemoryManager::Allocate(size, file, line, func);
}

// Override global new[] operator to track array allocations
inline void* operator new[](std::size_t size, const char* file, int line, const char* func)
{
    return Uma_Engine::MemoryManager::Allocate(size, file, line, func);
}

// Override global delete operator to track deallocations
void operator delete(void* ptr) noexcept
{
    Uma_Engine::MemoryManager::Free(ptr);
}

// Override global delete[] operator to track array deallocations
void operator delete[](void* ptr) noexcept
{
    Uma_Engine::MemoryManager::Free(ptr);
}

// Macro to get relative file path by removing project root
#define __RELFILE__ (__FILE__ + sizeof(PROJECT_ROOT) - 1)

// Macro to replace new with tracked version that captures file, line, and function
#define DEBUG_NEW new(__RELFILE__, __LINE__, __func__)
#define new DEBUG_NEW

#endif // DEBUG_MEM