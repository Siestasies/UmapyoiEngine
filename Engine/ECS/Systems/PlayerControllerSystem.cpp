/*!
\file   PlayerControllerSystem.cpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Jedrek Lee Jing Wei (50%), Leong Wai Men (50%)
\par    E-mail: jedrekjingwei.lee@digipen.edu, waimen.leong@digipen.edu
\par    DigiPen login: jedrekjingwei.lee@digipen.edu, waimen.leong

\brief
Implements player input handling system that translates keyboard events into character movement and actions.

Subscribes to KeyPress/Release/Repeat events and maintains input state for WASD movement, dash, attack, and interact actions.
Applies smooth acceleration to RigidBody based on input state to prevent jerky movement transitions.
Uses static boolean flags to detect single-press actions (attack, interact, dash) and emits PlayerActionEvents
through EventSystem. Includes conditional debug logging for action triggers. Movement applies directional acceleration
with configurable smoothing factor for responsive feel.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "PlayerControllerSystem.hpp"

#include "Components/RigidBody.h"
#include "Components/Transform.h"
#include "Components/Player.h"
#include "Components/Projectile.h"

#include "Debugging/Debugger.hpp"

#include "Math/Math.h"

// events
#include "Events/PlayerEvents.h"
#include "Events/CollisionEvent.h"

#include <GLFW/glfw3.h>

//#define _DEBUG_LOG

#ifdef _DEBUG_LOG
#include <iostream>
#endif // _DEBUG_LOG

namespace Uma_ECS
{
    void PlayerControllerSystem::Init(Uma_Engine::EventSystem* es, Uma_Engine::HybridInputSystem* is, Coordinator* c, Uma_Engine::Graphics* g)
    {
        pEventSystem = es;
        pHybridInputSystem = is;
        pCoordinator = c;
        pGraphicsSystem = g;

        SubscribeToEvents();
    }

    void PlayerControllerSystem::Update(float dt)
    {
        if (aEntities.empty()) return;

        if (!pCoordinator->IsActiveInHierarchy(aEntities[0]))
            return;
        auto& tf = pCoordinator->GetComponent<Transform>(aEntities[0]);
        auto& player = pCoordinator->GetComponent<Player>(aEntities[0]);

        if (!player.combatState.isAlive) return;

        // by right shd only have 1 player
        HandleMovementInput(dt);
        HandleActionInput(dt);
        HandlePlayerAnimation();
    }

    void PlayerControllerSystem::SubscribeToEvents()
    {
        pEventSystem->Subscribe<Uma_Engine::KeyPressEvent, PlayerControllerSystem>([this](const Uma_Engine::KeyPressEvent& e) { OnKeyPress(e); });
        pEventSystem->Subscribe<Uma_Engine::KeyReleaseEvent, PlayerControllerSystem>([this](const Uma_Engine::KeyReleaseEvent& e) { OnKeyRelease(e); });
        pEventSystem->Subscribe<Uma_Engine::KeyRepeatEvent, PlayerControllerSystem>([this](const Uma_Engine::KeyRepeatEvent& e) { OnKeyRepeat(e); });

        // Subscribe to mouse button events for fixed timestep compatibility
        pEventSystem->Subscribe<Uma_Engine::MouseButtonEvent, PlayerControllerSystem>([this](const Uma_Engine::MouseButtonEvent& e) {
            if (e.button == GLFW_MOUSE_BUTTON_2 && e.action == GLFW_PRESS) {
                inputState.rightMousePressed = true;
                inputState.rightMouseConsumed = false;  // Reset consumed flag on new press
            }
        });

        pEventSystem->Subscribe<Uma_Engine::MouseButtonEvent, PlayerControllerSystem>([this](const Uma_Engine::MouseButtonEvent& e) {
            if (e.button == GLFW_MOUSE_BUTTON_2 && e.action == GLFW_RELEASE) {
                inputState.rightMousePressed = false;
            }
        });

        // collision
        pEventSystem->Subscribe<Uma_Engine::OnCollisionEnterEvent, PlayerControllerSystem>([this](const Uma_Engine::OnCollisionEnterEvent& e) 
            { 
                (void)e;
                // do nth yet
            });
        pEventSystem->Subscribe<Uma_Engine::OnCollisionEvent, PlayerControllerSystem>([this](const Uma_Engine::OnCollisionEvent& e)
            {
                (void)e;
                // do nth yet
            });
        pEventSystem->Subscribe<Uma_Engine::OnCollisionExitEvent, PlayerControllerSystem>([this](const Uma_Engine::OnCollisionExitEvent& e)
            {
                (void)e;
                // do nth yet
            });

        pEventSystem->Subscribe<Uma_Engine::OnTriggerEnterEvent, PlayerControllerSystem>([this](const Uma_Engine::OnTriggerEnterEvent& e)
            {
                if (!pCoordinator || aEntities.empty()) return;
                if (e.entity != aEntities[0] && e.trigger != aEntities[0]) return;

                // proccess trigger

                Entity trigger = (e.entity != aEntities[0]) ? e.entity : e.trigger;

                HandleCollision(aEntities[0], trigger);
            });
        pEventSystem->Subscribe<Uma_Engine::OnTriggerEvent, PlayerControllerSystem>([this](const Uma_Engine::OnTriggerEvent& e)
            {
                (void)e;
                // do nth yet
            });
        pEventSystem->Subscribe<Uma_Engine::OnTriggerExitEvent, PlayerControllerSystem>([this](const Uma_Engine::OnTriggerExitEvent& e)
            {
                (void)e;
                // do nth yet
            });
    }

    void PlayerControllerSystem::HandlePlayerAnimation()
    {
        if (pCoordinator->HasComponent<Animator>(aEntities[0]))
        {
            auto& animator = pCoordinator->GetComponent<Animator>(aEntities[0]);
            auto& tf = pCoordinator->GetComponent<Transform>(aEntities[0]);
            auto& rb = pCoordinator->GetComponent<RigidBody>(aEntities[0]);
            auto& player = pCoordinator->GetComponent<Player>(aEntities[0]);
            auto& collider = pCoordinator->GetComponent<Collider>(aEntities[0]);
            auto& pf = pCoordinator->GetComponent<PathFinding>(aEntities[0]);

            if (rb.velocity.x < 0) tf.scale.x = -abs(tf.scale.x);
            if (rb.velocity.x > 0) tf.scale.x = abs(tf.scale.x);

            // State transition logic
            float velocityMagnitude = Uma_Math::magnitude(rb.velocity);

            // If moving and in idle state, transition to run
            if (velocityMagnitude > 0.1f && player.animatorState == PS_Idle)
            {
                player.animatorState = PS_Run;  // Assignment, not comparison
            }
            // If stopped and in run state, transition to idle
            else if (velocityMagnitude < 0.1f && player.animatorState == PS_Run)
            {
                player.animatorState = PS_Idle;  // Assignment, not comparison
            }
            // If attack animation finished, return to appropriate state
            else if ((player.animatorState == PS_Atk_1 || player.animatorState == PS_Atk_2)
                && animator.animator.HasFinished())
            {
                // Return to run or idle based on current velocity
                player.animatorState = (velocityMagnitude > 0.1f) ? PS_Run : PS_Idle;
                collider.shapes[2].isActive = false;
            }
            else if (player.animatorState == PS_Hurt && animator.animator.HasFinished())
            {
                player.animatorState = (velocityMagnitude > 0.1f) ? PS_Run : PS_Idle;
                collider.shapes[2].isActive = false;
            }

            if (player.mHealth <= 0)
            {
                player.animatorState = PS_Die;
            }
            else if (player.mHealth > 0 && !player.combatState.isAlive)
            {
                player.combatState.isAlive = true;
            }

            switch (player.animatorState)
            {
            case PS_Idle:
            {
                if (animator.animator.GetCurrentClip() != "idle")
                {
                    animator.animator.Play("idle", true);
                }
                break;
            }
            case PS_Run:
            {
                if (animator.animator.GetCurrentClip() != "run")
                {
                    animator.animator.Play("run", true);
                }
                break;
            }
            case PS_Atk_1:
            {
                if (animator.animator.GetCurrentClip() != "atk_1")
                {
                    animator.animator.Play("atk_1", true); 
                    collider.shapes[2].isActive = true;
                }
                break;
            }
            case PS_Atk_2:
            {
                if (animator.animator.GetCurrentClip() != "atk_2")
                {
                    animator.animator.Play("atk_2", true);
                    collider.shapes[2].isActive = true;
                }
                break;
            }
            case PS_Hurt:
            {
                if (animator.animator.GetCurrentClip() != "hurt")
                {
                    animator.animator.Play("hurt", true);
                }
                break;
            }
            case PS_Die:
            {
                if (animator.animator.GetCurrentClip() != "die")
                {
                    animator.animator.Play("die", true);
                }

                pf.reachedGoal = true;
                player.combatState.isAlive = false;

                break;
            }
            default:
                break;
            }

           
        }
    }

    void PlayerControllerSystem::HandleCollision(Entity deffender, Entity attacker)
    {
        if (pCoordinator->HasComponent<Projectile>(attacker))
        {
            // its a projectile 
            auto& projectile = pCoordinator->GetComponent<Projectile>(attacker);

            OnHurt(deffender, projectile.mDamage);

            pCoordinator->DestroyEntityAndChildren(attacker);
        }
    }

    void PlayerControllerSystem::OnHurt(Entity entity, int damage)
    {
        auto& player = pCoordinator->GetComponent<Player>(entity);

        player.mHealth -= damage - player.mDefense;

        player.animatorState = PS_Hurt;
    }

    void PlayerControllerSystem::Shutdown()
    {
        pEventSystem->UnsubscribeSystem<PlayerControllerSystem>();
    }

    void PlayerControllerSystem::OnKeyPress(const Uma_Engine::KeyPressEvent& event)
    {
        switch (event.key)
        {
        case GLFW_KEY_Q:
            inputState.attack_1 = true;
            break;
        case GLFW_KEY_W:
            inputState.attack_2 = true;
            break;
        case GLFW_KEY_E:
            inputState.interactPressed = true;
            break;
        case GLFW_KEY_LEFT_SHIFT:
            inputState.dashPressed = true;
            break;
        }
    }
    void PlayerControllerSystem::OnKeyRelease(const Uma_Engine::KeyReleaseEvent& event)
    {
        switch (event.key)
        {
        case GLFW_KEY_Q:
            inputState.attack_1 = false;
            break;
        case GLFW_KEY_W:
            inputState.attack_2 = false;
            break;
        case GLFW_KEY_E:
            inputState.interactPressed = false;
            break;
        case GLFW_KEY_LEFT_SHIFT:
            inputState.dashPressed = false;
            break;
        }
    }

    void PlayerControllerSystem::OnKeyRepeat(const Uma_Engine::KeyRepeatEvent& event)
    {
        (void)event;
    }

    void PlayerControllerSystem::HandleMovementInput(float dt)
    {
        (void)dt;
        if (aEntities.empty() || !pCoordinator->HasComponent<PathFinding>(aEntities[0])) return;

        // Use event-based input instead of state polling for fixed timestep compatibility
        // Only process if not consumed (single-press detection)
        if (inputState.rightMousePressed && !inputState.rightMouseConsumed)
        {
            auto& pf = pCoordinator->GetComponent<PathFinding>(aEntities[0]);
            pf.goal = pGraphicsSystem->ScreenToWorld(pHybridInputSystem->GetSceneMousePosition());
            pf.pathUpdateTimer = pf.pathUpdateInterval + 0.1f; // Force immediate update

            inputState.rightMouseConsumed = true;  // Mark as consumed
        }
    }

    void PlayerControllerSystem::HandleActionInput(float dt)
    {
        if (aEntities.empty()) return;

        static bool lastInteractState = false;
        static bool lastDashState = false;

        auto& player = pCoordinator->GetComponent<Player>(aEntities[0]);

        if (player.combatState.attack_1_cd_curr > 0.f)
        {
            player.combatState.attack_1_cd_curr -= dt;

            if (player.combatState.attack_1_cd_curr <= 0)
            {
                player.combatState.attack_1_cd_curr = 0.f;
                player.combatState.attack_1_is_in_cd = false;
            }
        }

        if (player.combatState.attack_2_cd_curr > 0.f)
        {
            player.combatState.attack_2_cd_curr -= dt;
            if (player.combatState.attack_2_cd_curr <= 0)
            {
                player.combatState.attack_2_cd_curr = 0.f;
                player.combatState.attack_2_is_in_cd = false;
            }
        }

        // Use event-based input instead of state polling for fixed timestep compatibility
        static bool lastAttack1State = false;
        static bool lastAttack2State = false;

        // Detect rising edge (just pressed) to avoid repeated triggers
        if (inputState.attack_1 && !lastAttack1State && !player.combatState.attack_1_is_in_cd)
        {
            //auto& animator = pCoordinator->GetComponent<Animator>(aEntities[0]);
            //animator.animator.Play("atk_1", true);
            player.animatorState = PS_Atk_1;

            player.combatState.attack_1_cd_curr = player.combatState.attack_1_cd;
            player.combatState.attack_1_is_in_cd = true;
        }

        // Attack 2 support (W key)
        if (inputState.attack_2 && !lastAttack2State && !player.combatState.attack_2_is_in_cd)
        {
            // Implement attack_2 logic here if needed
            //auto& animator = pCoordinator->GetComponent<Animator>(aEntities[0]);
            //animator.animator.Play("atk_2", true);
            player.animatorState = PS_Atk_2;

            player.combatState.attack_2_cd_curr = player.combatState.attack_2_cd;
            player.combatState.attack_2_is_in_cd = true;
        }

        lastAttack1State = inputState.attack_1;
        lastAttack2State = inputState.attack_2;
    }
}