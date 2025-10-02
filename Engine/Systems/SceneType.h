#pragma once
#include <RapidJSON/document.h>
#include <RapidJSON/writer.h>
#include <RapidJSON/stringbuffer.h>

namespace Uma_Engine
{
    class Scene
    {
        public:
            virtual void OnLoad() = 0;      // Load resources, setup entities
            virtual void OnUnload() = 0;    // Cleanup
            virtual void Update(float dt) = 0;
            virtual void Render() = 0;

            virtual ~Scene() = default;
    };
}