#pragma once

#include <functional>

#include "EventType.h"
#include "../ECS/Core/Types.hpp"

namespace Uma_Engine
{
	enum class EntityTemplate
	{
		Empty,
		Player,
		Enemy,
		Projectile,
		Particle
		// Add-on
	};

	class SpawnEntityRequestEvent : public Event
	{
	public:
		SpawnEntityRequestEvent(std::function<void(Uma_ECS::Entity)> setupCallback = nullptr) : setupCallback(setupCallback) { priority = Priority::Normal; }
	public:
		std::function<void(Uma_ECS::Entity)> setupCallback = nullptr;
	};

	class DestroyEntityRequestEvent : public Event
	{
	public:
		DestroyEntityRequestEvent(Uma_ECS::Entity entityId) : entityId(entityId) { priority = Priority::High; }
	public:
		Uma_ECS::Entity entityId;
	};

	class CloneEntityRequestEvent : public Event
	{
	public:
		CloneEntityRequestEvent(Uma_ECS::Entity entityId) : entityId(entityId) { priority = Priority::Normal; }

	public:
		Uma_ECS::Entity entityId;
	};

	class AddComponentRequestEvent : public Event
	{
	public:
		AddComponentRequestEvent(Uma_ECS::Entity entityId, std::type_index componentType) : entityId(entityId), componentType(componentType) { priority = Priority::Normal; }

	public:
		Uma_ECS::Entity entityId;
		std::type_index componentType;
	};

	class RemoveComponentRequestEvent : public Event
	{
	public:
		RemoveComponentRequestEvent(Uma_ECS::Entity entityId, std::type_index componentType) : entityId(entityId), componentType(componentType) { priority = Priority::Normal; }

	public:
		Uma_ECS::Entity entityId;
		std::type_index componentType;
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