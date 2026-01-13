#include "../Core/System.hpp"
#include "../Core/Coordinator.hpp"
#include "../Core/EventSystem.h"

namespace Uma_ECS
{
    class FSMSystem : public ECSSystem
    {
    public:
        
        void Update(float dt);
    };
}