#pragma once

#include "ECS/Core/Types.hpp"
#include <memory>


// component
namespace Uma_ECS 
{
    struct State {
        virtual ~State() = default;
        virtual void enter(Entity) {}
        virtual void exit(Entity) {}
        virtual void update(Entity, float dt) = 0;
    };

    struct FSM{
        std::shared_ptr<State> currState;

        void Update(Entity entity ,float dt) {
            if (currState) {
                currState->update(entity, dt);
            }
        }

        //calling changeState<new state struct>
        template<typename NewState>
        void ChangeState(Entity entity) {
            //exits the current state if any
            if (currState) {
                currState->exit(entity);
            }
            //creates new state with unique pointer
            currState = std::make_shared<NewState>();
            //enters the new state
            currState->enter(entity);
        }
        
        void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) 
        {
        }

        // Deserialize from JSON
        void Deserialize(const rapidjson::Value& value)
        {
        }
    };

    //how to use?
    /*
    struct idleState : State<*insert entity type here>{
    //populate as needed
    void enter
    void update
    void exit
    */

}

