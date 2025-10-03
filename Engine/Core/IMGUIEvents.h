/*!
\file   IMGUIEvents.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Jedrek Lee Jing Wei (100%)
\par    E-mail: jedrekjingwei.lee@digipen.edu
\par    DigiPen login: jedrekjingwei.lee

\brief
Defines UI-related events for handling IMGUI interactions within the Uma Engine.

Includes event types for common UI requests such as spawning or destroying entities,
component modifications, scene management actions, and querying active entities.
Each event encapsulates relevant data and uses priority levels to control event processing
within the IMGUI event handling system.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include <string>
#include <vector>

#include "EventType.h"
#include "../ECS/Core/Types.hpp"

namespace Uma_Engine
{
	class SpawnEntityRequestEvent : public Event
	{
	public:
		SpawnEntityRequestEvent() { priority = Priority::Normal; }
	};

	class DestroyEntityRequestEvent : public Event
	{
	public:
		DestroyEntityRequestEvent(Uma_ECS::Entity entityId) : entityId(entityId) { priority = Priority::High; }
	public:
		Uma_ECS::Entity entityId;
	};

	class StressTestRequestEvent : public Event
	{
	public:
			StressTestRequestEvent(){ priority = Priority::High; }
	};

	class ShowEntityInVPRequestEvent : public Event
	{
	public:
			ShowEntityInVPRequestEvent() { priority = Priority::High; }
	};

	class ChangeEnemyRotRequestEvent : public Event
	{
	public:
			float rot;
			ChangeEnemyRotRequestEvent(float rot) : rot(rot) { priority = Priority::High; }
	};

	class ChangeEnemyXposRequestEvent : public Event
	{
	public:
			float xpos;
			ChangeEnemyXposRequestEvent(float xpos) : xpos(xpos) { priority = Priority::High; }
	};

	class LoadPrefabRequestEvent : public Event
	{
	public:
			
			LoadPrefabRequestEvent() { priority = Priority::High; }
	};

	class ChangeEnemyScaleRequestEvent : public Event
	{
	public:
			float scale;
			ChangeEnemyScaleRequestEvent(float scale) : scale(scale) { priority = Priority::High; }
	};

	class ShowBBoxRequestEvent : public Event
	{
	public:
			bool show;
			ShowBBoxRequestEvent(bool show) : show(show) { priority = Priority::High; }
	};

	class CloneEntityRequestEvent : public Event
	{
	public:
		CloneEntityRequestEvent(Uma_ECS::Entity entityId) : entityId(entityId) { priority = Priority::High; }
	public:
		Uma_ECS::Entity entityId;
	};

	class QueryActiveEntitiesEvent : public Event
	{
	public:
		QueryActiveEntitiesEvent()
		{
			priority = Priority::Critical;
		}

	public:
		mutable int mActiveEntityCnt = 0;
		//mutable std::vector<Uma_ECS::Entity> entities;
	};

	class AddComponentRequestEvent : public Event
	{
	public:
		AddComponentRequestEvent(Uma_ECS::Entity entityId, std::string componentType) : entityId(entityId), componentType(componentType) { priority = Priority::Normal; }

	public:
		Uma_ECS::Entity entityId;
		std::string componentType;
	};

	class RemoveComponentRequestEvent : public Event
	{
	public:
		RemoveComponentRequestEvent(Uma_ECS::Entity entityId, std::string componentType) : entityId(entityId), componentType(componentType) { priority = Priority::Normal; }

	public:
		Uma_ECS::Entity entityId;
		std::string componentType;
	};

	class ClearSceneRequestEvent : public Event
	{
	public:
		ClearSceneRequestEvent() { priority = Priority::High; }
	};

	class SaveSceneRequestEvent : public Event
	{
	public:
		SaveSceneRequestEvent(const std::string& filepath = "") : filepath(filepath) { priority = Priority::Normal; }

	public:
		std::string filepath;
	};

	class LoadSceneRequestEvent : public Event
	{
	public:
		LoadSceneRequestEvent(const std::string& filepath) : filepath(filepath) { priority = Priority::High; }

	public:
		std::string filepath;
	};
}