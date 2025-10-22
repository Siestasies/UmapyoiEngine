#pragma once
#include <memory>
#include "../Core/SystemType.h"

template <typename Parent>
struct State {
    virtual ~State() = default;
    virtual void enter(Parent) {}
    virtual void exit(Parent) {}
    virtual void update(Parent, float dt) = 0;
};

namespace Uma_Engine {

    template <typename Parent>
    struct FSM{
    public:
        std::unique_ptr<State<Parent>> currState;

        void Update(float dt) {
            if (currState) {
                currState->update(*parent, dt);
            }
        }

        //calling changeState<new state struct>
        template<typename NewState>
        void ChangeState(Parent parent) {
            //exits the current state if any
            if (currState) {
                currState->exit(parent);
            }
            //creates new state with unique pointer
            currState = std::make_unique<NewState>();
            //enters the new state
            currState->enter(parent);
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

