#pragma once
#include <memory>

using Entity = Uma_ECS::Entity;

struct State {
    virtual ~State() = default;
    virtual void enter(Entity) {}
    virtual void exit(Entity) {}
    virtual void update(Entity, float dt) = 0;
};

namespace Uma_ECS {

    struct FSM{
        std::shared_ptr<State> currState;

        void Update(Entity entity ,float dt) {
            printf("a");
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
            currState = std::make_unique<NewState>();
            //enters the new state
            currState->enter(entity);
        }
        
        void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const //override
        {
        }

        // Deserialize from JSON
        void Deserialize(const rapidjson::Value& value) //override
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

