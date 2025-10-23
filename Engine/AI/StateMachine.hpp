#pragma once

#include "ECS/Core/Types.hpp"
#include <memory>
#include <unordered_map>
#include <string>
#include <iostream>



// component
namespace Uma_ECS 
{
    struct FSM;

    struct State {
        virtual ~State() = default;
        virtual void enter(FSM&) {}
        virtual void exit(FSM&) {}
        virtual void update(FSM&, float dt) = 0;
    };

    struct FSM{
        std::string currStateName;
        std::unordered_map<std::string, std::shared_ptr<State>> states;

        void Update(FSM& entity ,float dt) {
            if (auto it = states.find(currStateName); it != states.end()) {
                it->second->update(entity, dt);
            }
        }

        //calling changeState<new state struct>
        void ChangeState(FSM& entity,std::string name) {
            //exits the current state if any
            //if (auto it = states.find(currStateName); it != states.end()) {
            //    it->second->exit(entity);
            //    
            //}
            ////enters the new state
            //currStateName = name;
            //if (auto it = states.find(name); it != states.end()) {
            //    it->second->enter(entity);
            //}

            entity.currStateName = "IM A CHICKEN";
        }

        template <typename... NewStates>
        void registerState() {
            (registerOne<NewStates>(), ...);
        }

        template <typename StateType>
        void registerOne() {
            states.emplace(std::string(StateType::name), std::make_shared<StateType>());
        }
        
        void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) 
        {
        }

        // Deserialize from JSON
        void Deserialize(const rapidjson::Value& value)
        {
        }
    };

    struct base : State {
        static constexpr const char* name = "Base";
        float timer = 0;
        void enter(FSM& entity) override {
            //entity.currStateName = name;
        }
        void exit(FSM& entity) override {
            //entity.currStateName = "";
        }
        void update(FSM& entity, float dt) override {
            timer += dt;
            if (timer > 1) {
                //std::cout << timer << "\n";
                entity.ChangeState(entity, "Move");
                std::cout << "changed to move\n";
                std::cout << entity.currStateName << "\n";
            }
        }
    };

    struct move : State {
        static constexpr const char* name = "Move";
        void enter(FSM& entity) override {
            //entity.currStateName = name;
        }
        void exit(FSM& entity) override {
            //entity.currStateName = "";
        }
        void update(FSM& entity, float dt) override {
            std::cout << "move bitch\n";
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

