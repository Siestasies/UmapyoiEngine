#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include <type_traits>
#include <chrono>
#include <string>
#include <iostream>
#include "SystemType.h"
#include "../Systems/Window.hpp"

namespace Uma_Engine
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
            ptr->SetSystemManager(this);
            systems.push_back(std::move(system));
            timings.push_back(0.0); // keep timings vector in sync
            return ptr;
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
                if (auto windowSystem = dynamic_cast<IWindowSystem*>(system.get()))
                {
                    windowSystem->SetWindow(window);
                }
            }
        }

        // Update all systems with profiling
        void Update(float dt)
        {
            using namespace std::chrono;

            static double timeCheck = 0.0;
            static int frameCounter = 0;

            timeCheck += dt;
            frameCounter++;

            if (timeCheck >= 1.0)
            {
                double totalTime = 0.0;

                for (size_t i = 0; i < systems.size(); ++i)
                {
                    auto& system = systems[i];
                    // calc timing per system
                    auto start = high_resolution_clock::now();
                    system->Update(dt);
                    auto end = high_resolution_clock::now();

                    double elapsed = duration<double, std::milli>(end - start).count();
                    timings[i] = elapsed;
                    totalTime += elapsed;
                }
                lastTotalTime = totalTime;

                // reset
                timeCheck = 0.0;
                frameCounter = 0;
            }
            else
            {
                for (auto& system : systems)
                {
                    system->Update(dt);
                }
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

        // Access profiling info
        double GetLastTotalTime() const { return lastTotalTime; }
        const std::vector<double>& GetLastTimings() const { return timings; }

        std::string GetSystemName(size_t index)
        {
            return typeid(*systems[index].get()).name();
        }

    private:
        std::vector<std::unique_ptr<ISystem>> systems;
        std::vector<double> timings;
        double lastTotalTime = 0.0;
    };
}
