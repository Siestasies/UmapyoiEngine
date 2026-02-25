/*!
\file   FSMSystem.hpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Koh Kai Yang (100%)
\par    E-mail: k.kaiyang@digipen.edu
\par    DigiPen login: k.kaiyang

\brief
System that processes state changes for FSM

All content (C) 2026 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "../Core/System.hpp"
#include "../Core/Coordinator.hpp"
#include "../Core/EventSystem.h"
#include "FSM.h"

namespace Uma_ECS
{
    class FSMSystem : public ECSSystem
    {
    private:
        Coordinator* pCoordinator = nullptr;

    public:
        /*!
        * \brief passes reference of coordinator to this component update
        * \param coordinator pointer
        * \return nothing
        */
        void Init(Coordinator* c);
        
        /*!
        * \brief runs update for the states
        * \param delta time
        * \return nothing
        */
        void Update(float dt);
    };
}