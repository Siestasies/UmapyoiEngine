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

        void Init(Coordinator* c);
        
        void Update(float dt);
    };
}