#pragma once
/*!
\file   AudioComponent.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

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
	struct AudioComponent {
        std::string loopingSoundName;

        FMOD_VECTOR position = { 0.0f, 0.0f, 0.0f };
        FMOD_VECTOR velocity = { 0.0f, 0.0f, 0.0f };

        float volume = 1.0f;
        bool isPlaying = false;
        bool shouldLoop = true;

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