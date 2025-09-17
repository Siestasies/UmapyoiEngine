#pragma once

namespace Uma_Engine
{
    // foward declare of System Manager
    class SystemManager;

    // Base system interface - only requires basic lifecycle methods
    class ISystem
    {
    public:
        virtual ~ISystem() = default;

        /*!
            \brief Initialize the system.
            \details Called once at engine startup.
        */
        virtual void Init() = 0;

        /*!
            \brief Update the system each frame.
            \param dt Delta time in seconds since last frame.
        */
        virtual void Update(float dt) = 0;

        /*!
            \brief Clean up system resources.
            \details Called once at engine shutdown.
        */
        virtual void Shutdown() = 0;

        inline void SetSystemManager(SystemManager* sm) { pSystemManager = sm; }

    protected:
        SystemManager* pSystemManager = nullptr;
    };
}