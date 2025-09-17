#pragma once
#include "../Core/SystemType.h"
#include <string>
#include <unordered_map>

namespace Uma_Engine
{
    class Graphics;

    class ResourcesManager : public ISystem
    {
        void Init() override;

        void Update(float dt) override;

        void Shutdown() override;

    private:
        std::unordered_map<std::string, unsigned int> aTextures{};

        Graphics* gGraphics;
    };
}