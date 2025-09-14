#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include <type_traits>
#include "SystemType.h"

namespace UmapyoiEngine
{
    class SystemManager
    {
    public:
        // Adds a system to the manager
        template <typename T>
        T* RegisterSystem()
        {
            static_assert(std::is_base_of<ISystem, T>::value, "T must inherit from ISystem.");
            auto system = std::make_unique<T>();
            T* ptr = system.get();
            systems.push_back(std::move(system));
            return ptr; // Return pointer for further configuration if needed
        }

        // Initialize all systems
        void Init()
        {
            for (auto& system : systems)
            {
                system->Init();
            }
        }

        // Set window for systems that need it
        void SetWindow(GLFWwindow* window)
        {
            for (auto& system : systems)
            {
                // Check if system implements IWindowSystem
                if (auto windowSystem = dynamic_cast<IWindowSystem*>(system.get()))
                {
                    windowSystem->SetWindow(window);
                }
            }
        }

        // Update all systems
        void Update(float dt)
        {
            for (auto& system : systems)
            {
                system->Update(dt);
            }
        }

        // Shutdown all systems
        void Shutdown()
        {
            for (auto& system : systems)
            {
                system->Shutdown();
            }
        }

        // Get a system by type
        template<typename T>
        T* GetSystem()
        {
            for (auto& system : systems)
            {
                if (auto ptr = dynamic_cast<T*>(system.get()))
                {
                    return ptr;
                }
            }
            return nullptr;
        }

    private:
        std::vector<std::unique_ptr<ISystem>> systems;
    };
}