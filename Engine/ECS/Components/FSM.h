#pragma once
/*!
\file   FSM.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Koh Kai Yang (100%)
\par    E-mail: k.kaiyang@digipen.edu
\par    DigiPen login: k.kaiyang

\brief
component used for state machine on the entity

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "../Engine/ECS/Core/Types.hpp"
#include "unordered_map"

namespace Uma_ECS
{
	struct State
	{
		std::string name;
		bool isActive = true;
		int scriptIndex = -1;
	};

	struct FSM
	{
		//stores all states
		//std::vector<State> states;

		std::unordered_map<std::string, State> states;

		unsigned int nextId = 0;

		std::string current;
		std::string next;

        /*!
        * \brief passes in the name of the state to change to
        * \param name of the next state
        * \return bool if the next state is found and has changed to that state
        */
		bool ChangeStates(const std::string& name) {
			if (name.empty()) return false;
			auto it = states.find(name);
			//if name not found or if the script is inactive dont change to the state
			if (it == states.end() || !it->second.isActive) {
				//if state not found return
				//can log error if needed
				return false;
			}

			//to allow for exit/enter code to run before the transition
			next = name;
			return true;
		};

        /*!
        * \brief passes in a state to be added to the state list
        * \param name and is the script/state active
        * \return bool to indicate successful adding of state
        */
		bool AddStates(const std::string& name, bool isActive) {
			if (name.empty()) return false;

			std::string path = name;
			auto [iter, inserted] = states.emplace(name, State{ path, isActive, static_cast<int>(nextId) });
			if (!inserted) {
				return false;
			}
			++nextId;
			return true;
		}

        /*!
        * \brief serliaise the code from the scene file
        * \param json value and allocator
        * \return nothing
        */
        void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const
        {
            value.SetObject();

            // States array
            rapidjson::Value statesArray(rapidjson::kArrayType);
            for (const auto& [stateName, state] : states) {
                rapidjson::Value stateObj(rapidjson::kObjectType);
                stateObj.AddMember("name", rapidjson::Value(state.name.c_str(), allocator), allocator);
                stateObj.AddMember("isActive", state.isActive, allocator);
                stateObj.AddMember("scriptIndex", state.scriptIndex, allocator);
                statesArray.PushBack(stateObj, allocator);
            }
            value.AddMember("states", statesArray, allocator);

            // nextId (for stable indices if needed)
            value.AddMember("nextId", static_cast<int>(nextId), allocator);

            // current/next (transient but harmless backup)
            value.AddMember("current", rapidjson::Value(current.c_str(), allocator), allocator);
            value.AddMember("next", rapidjson::Value(next.c_str(), allocator), allocator);
        }

        /*!
        * \brief Deserialize the code to the scene file
        * \param json value and allocator
        * \return nothing
        */
        void Deserialize(const rapidjson::Value& value)
        {
            // Full reset
            states.clear();
            current.clear();
            next.clear();
            nextId = 0;

            // Restore states (uses your AddStates logic!)
            if (value.HasMember("states") && value["states"].IsArray()) {
                for (const auto& stateObj : value["states"].GetArray()) {
                    if (stateObj.IsObject() &&
                        stateObj.HasMember("name") && stateObj["name"].IsString() &&
                        stateObj.HasMember("isActive") && stateObj["isActive"].IsBool()) {

                        std::string name = stateObj["name"].GetString();
                        AddStates(name, stateObj["isActive"].GetBool());  // Increments nextId!

                        // Override saved index (stable!)
                        if (stateObj.HasMember("scriptIndex") && stateObj["scriptIndex"].IsInt()) {
                            states[name].scriptIndex = stateObj["scriptIndex"].GetInt();
                        }
                    }
                }
            }

            // Restore counter
            if (value.HasMember("nextId") && value["nextId"].IsInt()) {
                nextId = static_cast<unsigned int>(value["nextId"].GetInt());
            }

            // Restore transients (FSMSystem will fix if invalid)
            if (value.HasMember("current") && value["current"].IsString()) {
                current = value["current"].GetString();
            }
            if (value.HasMember("next") && value["next"].IsString()) {
                next = value["next"].GetString();
            }
        }

	};

}