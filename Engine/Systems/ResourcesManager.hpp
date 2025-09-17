#pragma once
#include "../Core/SystemType.h"
#include "Math/Math.hpp"
#include "ResourcesTypes.hpp"

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

        void Load_Texture(const std::string& texname, const std::string& filename);

    private:
        std::unordered_map<std::string, Texture> aTextures{};

        Graphics* gGraphics;
    };
}