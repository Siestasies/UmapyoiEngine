#pragma once
/*!
\file   AudioListener.h
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A
\par    Software Engineering Project 4

\author Koh Kai Yang (100%)
\par    E-mail: k.kaiyang@digipen.edu
\par    DigiPen login: k.kaiyang

\brief
Adds the component tag to the entity so the system can use this entity to update the audio listener

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

namespace Uma_ECS
{
    /*!
    * \brief Adds the audio listener component to tag the player to set 3d audio listener
    */
	struct AudioListener {
		//empty on purpose this is js to tag the listener

        /*!
        * \brief this is to serialize the info from json
        * \param value, allocator 
        * \return nothing
        */
        void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const //override
        {
            (void)value;
            (void)allocator;
        }

        /*!
        * \brief this is to deserialize the info into json
        * \param value
        * \return nothing
        */
        void Deserialize(const rapidjson::Value& value) //override
        {
            (void)value;
        }
	};
}