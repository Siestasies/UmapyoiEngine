/*!
\file   ECS_Components.hpp
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A
\par    Software Engineering Project 4

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Convenience header that aggregates all ECS component and UI component includes
into a single file for unified access throughout the engine.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

// ECS Components
#include "Animator.h"
#include "AudioComponent.h"
#include "AudioListener.h"
#include "Camera.h"
#include "Collider.h"
#include "Enemy.h"
#include "FSM.h"
#include "LuaScript.h"
#include "ParticleEmitter.h"
#include "PathFinding.h"
#include "Player.h"
#include "Prefab.h"
#include "Projectile.h"
#include "RigidBody.h"
#include "Sprite.h"
#include "Transform.h"

// UI Components
#include "UI/Components/Button.h"
#include "UI/Components/Canvas.h"
#include "UI/Components/Image.h"
#include "UI/Components/RectTransform.h"
#include "UI/Components/Text.h"