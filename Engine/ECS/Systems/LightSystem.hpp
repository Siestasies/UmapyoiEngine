/*!
\file   LightSystem.hpp
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A

\author Javier Chua Dong Qing (100%)
\par    E-mail: javierdongqing.chua@digipen.edu
\par    DigiPen login: javierdongqing.chua

\brief
Renders a multiplicative 2D light map from Light2D components.

Inserted between the world sprite pass and UI pass so that world geometry
is darkened/lit but UI elements remain unaffected.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "../Core/System.hpp"
#include "../../Math/Math.h"

namespace Uma_ECS { class Coordinator; }
namespace Uma_Engine { class Graphics; }

namespace Uma_ECS
{
    class LightSystem : public ECSSystem
    {
    public:
        void Init(Uma_Engine::Graphics* graphics, Coordinator* coordinator);
        void Update(float dt);

        // Scene ambient color (darkness level when no lights reach an area)
        Vec3 ambientColor = Vec3(0.1f, 0.1f, 0.15f);

    private:
        Coordinator* pCoordinator = nullptr;
        Uma_Engine::Graphics* pGraphics = nullptr;
    };
}
