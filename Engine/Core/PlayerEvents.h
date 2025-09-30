#pragma once

#include "EventType.h"
#include "../ECS/Core/Types.hpp"

namespace Uma_Engine
{
    // Player movement event - emitted when player velocity changes
    class PlayerMoveEvent : public Event
    {
    public:
        PlayerMoveEvent(Uma_ECS::Entity playerId, float velocityX, float velocityY)
            : playerId(playerId), velocityX(velocityX), velocityY(velocityY)
        {
            priority = Priority::Normal;
        }

    public:
        Uma_ECS::Entity playerId;
        float velocityX, velocityY;
    };

    // Player action event - emitted for discrete player actions
    class PlayerActionEvent : public Event
    {
    public:
        enum class ActionType
        {
            //Jump,
            Attack,
            Interact,
            Dash,
            Block,
            UseItem,
            Crouch,
            Sprint
        };

        PlayerActionEvent(Uma_ECS::Entity playerId, ActionType action)
            : playerId(playerId), action(action)
        {
            priority = Priority::High;
        }

    public:
        Uma_ECS::Entity playerId;
        ActionType action;
    };

    // Player state change event - for major state transitions
    class PlayerStateChangeEvent : public Event
    {
    public:
        enum class StateType
        {
            Idle,
            Moving,
            //Jumping,
            Falling,
            Attacking,
            Dead,
            Invulnerable,
            Stunned
        };

        PlayerStateChangeEvent(Uma_ECS::Entity playerId, StateType fromState, StateType toState)
            : playerId(playerId), fromState(fromState), toState(toState)
        {
            priority = Priority::High;
        }

    public:
        Uma_ECS::Entity playerId;
        StateType fromState, toState;
    };

    // Player health change event
    class PlayerHealthChangeEvent : public Event
    {
    public:
        PlayerHealthChangeEvent(Uma_ECS::Entity playerId, float oldHealth, float newHealth, float maxHealth)
            : playerId(playerId), oldHealth(oldHealth), newHealth(newHealth), maxHealth(maxHealth)
        {
            priority = Priority::High;
        }

    public:
        Uma_ECS::Entity playerId;
        float oldHealth, newHealth, maxHealth;
    };

    // Player spawn/respawn event
    class PlayerSpawnEvent : public Event
    {
    public:
        PlayerSpawnEvent(Uma_ECS::Entity playerId, float x, float y, bool isRespawn = false)
            : playerId(playerId), x(x), y(y), isRespawn(isRespawn)
        {
            priority = Priority::High;
        }

    public:
        Uma_ECS::Entity playerId;
        float x, y;
        bool isRespawn;
    };

    // Player death event
    class PlayerDeathEvent : public Event
    {
    public:
        PlayerDeathEvent(Uma_ECS::Entity playerId, const std::string& causeOfDeath = "")
            : playerId(playerId), causeOfDeath(causeOfDeath)
        {
            priority = Priority::Critical;
        }

    public:
        Uma_ECS::Entity playerId;
        std::string causeOfDeath;
    };
}