/*!
\file   SystemManager.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Shahir Rasid (100%)
\par    E-mail: b.muhammadshahir@digipen.edu
\par    DigiPen login: b.muhammadshahir

\brief
This file implements the definition for a System manager which stores
and controls the life-cycle of a system.
Also contains some helper functions to calculate time per system.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/
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
    /*!
     * \class SystemManager
     * \brief Manages the lifecycle and per-frame updating of all engine systems.
     */
    class SystemManager
    {
    public:
        /*!
         * \brief Registers and constructs a new system of the given type.
         * \return Pointer to the newly created system.
         */
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

        /*!
         * \brief Initializes all registered systems in order.
         */
        void Init()
        {
            for (auto& system : systems)
            {
                system->Init();
            }
        }

        /*!
         * \brief Passes the GLFW window handle to all systems that implement IWindowSystem.
         * \param window Pointer to the GLFW window.
         */
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

        /*!
         * \brief Updates all systems. Profiles per-system timing once per second.
         * \param dt Delta time in seconds.
         */
        void Update(float dt)
        {
            static double timeCheck = 0.0;
            static int frameCounter = 0;

            timeCheck += dt;
            frameCounter++;

            if (timeCheck >= 1.0)
            {
                double totalTime = 0.0;

                // loop thru all systems and check timing per system
                for (size_t i = 0; i < systems.size(); ++i)
                {
                    auto& system = systems[i];

                    // calc timing per system
                    auto start = std::chrono::high_resolution_clock::now();
                    system->Update(dt);
                    auto end = std::chrono::high_resolution_clock::now();

                    double elapsed = std::chrono::duration<double, std::milli>(end - start).count();
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

        /*!
         * \brief Shuts down all registered systems in order.
         */
        void Shutdown()
        {
            for (auto& system : systems)
            {
                system->Shutdown();
            }
        }

        /*!
         * \brief Retrieves a registered system by its type.
         * \return Pointer to the system, or nullptr if not found.
         */
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

        /*!
         * \brief Returns the total profiled update time from the last measurement interval.
         * \return Total time in milliseconds.
         */
        double GetLastTotalTime() const { return lastTotalTime; }

        /*!
         * \brief Returns per-system profiled timings from the last measurement interval.
         * \return Vector of per-system durations in milliseconds.
         */
        const std::vector<double>& GetLastTimings() const { return timings; }

        /*!
         * \brief Returns the demangled type name of a system at the given index.
         * \param index Index of the system in registration order.
         * \return System type name string.
         */
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
