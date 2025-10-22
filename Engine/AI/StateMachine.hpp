#pragma once
#include <memory>
#include "../Core/SystemType.h"

template <typename Owner>
struct State {
    virtual ~State() = default;
    virtual void enter(Owner&) {}
    virtual void exit(Owner&) {}
    virtual void update(Owner&, float dt) = 0;
};

namespace Uma_Engine {

    template <typename Owner>
    class FSM : public ISystem {
    public:
        explicit FSM(Owner* ownerPtr) : owner(ownerPtr) {}

        void Init() override {}
        void Shutdown() override {}
        void Update(float dt) override {
            if (currState) {
                currState->update(*owner, dt);
            }
        }

        template<typename NewState, typename... Args>
        void ChangeState(Args&&... args) {
            if (currState) {
                currState->exit(*owner);
            }
            currState = std::make_unique<NewState>(std::forward<Args>(args)...);
            currState->enter(*owner);
        }

    private:
        Owner* owner = nullptr;
        std::unique_ptr<State<Owner>> currState;
    };

}

