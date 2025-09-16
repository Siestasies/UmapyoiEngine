#pragma once

#include "../Core/SystemType.h"

namespace Uma_Engine
{
    class Test_Graphics : public ISystem
    {
        void Init() override;
        void Update(float dt) override;
        void Shutdown() override;
    };
}