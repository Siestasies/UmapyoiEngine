#pragma once

namespace Uma_Engine
{
    class Scene
    {
        public:
            virtual void OnLoad() {}      // Load resources, setup entities
            virtual void OnUnload() {}    // Cleanup
            virtual void Update(float dt) = 0;
            virtual void Render() = 0;
    };
}