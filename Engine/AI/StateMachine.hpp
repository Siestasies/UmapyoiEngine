#pragma once
#include "memory"
#include "../Core/SystemType.h"

template <typename Owner>
struct State {
	virtual ~State() = default;
	virtual void enter(Owner&){}
	virtual void exit(Owner&){}
	virtual void update(Owner&,float dt) = 0;
};

namespace Uma_Engine{

	template <typename Owner>
	class FSM : public ISystem {
	public:
		FSM(Owner* ownerPtr) : owner(ownerPtr) {}

		// Initialize and cleanup
		void Init() override{

		}
		void Shutdown() override {

		}
		void Update(float dt) override {
			if (currState) {
			}
		}

	private:
		Owner* owner;
		//using unique ptr because the state machine will only ever have 
		//1 state running at one time so the safest way to use the states is
		//to make sure there is only 1
		std::unique_ptr<State<Owner>> currState;
	};

};

