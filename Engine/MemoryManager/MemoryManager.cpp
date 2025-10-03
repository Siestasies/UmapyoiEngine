/*!
\file   MemoryManager.cpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3
\author Lai Jun Siang(100%)
\par    E-mail: lai.j@digipen.edu
\par    DigiPen login: lai.j
\brief
Implementation of the memory management and tracking system for debugging memory leaks.
This file contains the core functionality for custom memory allocation and deallocation
with comprehensive tracking capabilities. The system maintains a registry of all active
allocations with metadata including size, source location, and function context. It
provides detailed logging of allocation and deallocation operations, detects memory
leaks by identifying unfreed allocations, and generates comprehensive leak reports
with source code locations to aid in debugging memory-related issues.
All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "MemoryManager.hpp"
#include "../Debugging/Debugger.hpp"
#include <cstdlib>
#include <new>

namespace Uma_Engine
{

    // Static member initialization
    bool MemoryManager::sEnableLogs = false;
    bool MemoryManager::sEnableTracking = false;
    std::unordered_map<void*, MemoryManager::AllocInfo> MemoryManager::sMemAllocs;

    void* MemoryManager::Allocate(std::size_t size, const char* file, int line, const char* func)
    {
        // Attempt to allocate requested memory
        void* ptr = std::malloc(size);
        if (!ptr)
        {
            // Log allocation failure with full context
            Debugger::Log(WarningLevel::eError,
                "Bad Alloc of " + std::to_string(size) + " bytes at " + file +
                ", line " + std::to_string(line) + " in \"" + func + "\"");

            // Throw standard exception for allocation failure
            throw std::bad_alloc();
        }

        // If tracking is disabled, return pointer without recording
        if (!sEnableTracking)
            return ptr;

        // Store allocation metadata for leak detection
        sMemAllocs[ptr] = { size, file, func, line };

        // Optionally log successful allocation
        if (sEnableLogs)
            Debugger::Log(WarningLevel::eInfo,
                "Allocated " + std::to_string(size) + " bytes at " + file +
                ", line " + std::to_string(line) + " in \"" + func + "\"");

        return ptr;
    }

    void MemoryManager::Free(void* ptr)
    {
        // If tracking is disabled, directly free memory
        if (!sEnableTracking)
        {
            std::free(ptr);
            return;
        }

        // Search for allocation in tracking map
        std::unordered_map<void*, MemoryManager::AllocInfo>::const_iterator it;
        if ((it = sMemAllocs.find(ptr)) != sMemAllocs.end())
        {
            // Optionally log deallocation with original allocation info
            if (sEnableLogs)
                Debugger::Log(WarningLevel::eInfo,
                    "Freed " + std::to_string(it->second.size) + " bytes from " + it->second.file +
                    ", line " + std::to_string(it->second.line) + " in \"" + it->second.func + "\"");

            // Remove from tracking map
            sMemAllocs.erase(it);
        }

        // Free the actual memory
        std::free(ptr);
    }

    void MemoryManager::ReportLeaks()
    {
        // Check if any allocations remain unfreed
        if (!sMemAllocs.empty())
        {
            Debugger::Log(WarningLevel::eError, "=== Memory leaks Detected ===");

            // Report each leaked allocation with its metadata
            for (auto& [ptr, info] : sMemAllocs)
            {
                Debugger::Log(WarningLevel::eError,
                    "Leak of " + std::to_string(info.size) + " bytes from " + info.file +
                    ", line " + std::to_string(info.line) + " in \"" + info.func + "\"");
            }
        }
        else
        {
            // Report clean shutdown with no leaks
            Debugger::Log(WarningLevel::eInfo, "=== No Memory leaks Detected ===");
        }
    }

    void MemoryManager::Enable()
    {
        // Enable allocation tracking
        sEnableTracking = true;
    }

    void MemoryManager::Disable()
    {
        // Disable allocation tracking
        sEnableTracking = false;
    }

} // namespace Uma_Engine