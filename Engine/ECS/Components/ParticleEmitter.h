/*!
\file    ParticleEmitter.h
\par     Project: GAM200
\par     Course: CSD2401
\par     Section A
\par     Software Engineering Project 3

\author Javier Chua Dong Qing (100%)
\par     E-mail: javierdongqing.chua@digipen.edu
\par     DigiPen login: javierdongqing.chua

\brief
This file contains the configuration structs for particle behavior and runtime containers.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once
#include <vector>
#include <string>
#include <rapidjson/document.h>

namespace Uma_ECS
{
    /*!
    \enum EmitterMode
    \brief Defines how particles are generated over time.
    */
    enum class EmitterMode
    {
        Burst,        // One-time burst at emitter position
        Continuous,   // Constant emission at emitter position
        ScreenFill    // Fill entire viewport (like for example for snowing effect on screen)
    };

    /*!
    \struct Particle
    \brief Represents the runtime state of a single particle.
    */
    struct Particle
    {
        Vec2 position = { 0, 0 };
        Vec2 velocity = { 0, 0 };
        Vec3 color = { 1, 1, 1 };
        float scale = 1.0f;
        float rotation = 0.0f;
        float rotationSpeed = 0.0f; // Degrees per second
        float lifetime = 0.0f;
        float maxLifetime = 1.0f;
        float age = 0.0f;           // How long particle has been alive
        float opacity = 1.0f;
        float baseOpacity = 1.0f;   // Target opacity before fade
        bool active = false;
    };

    /*!
    \struct ParticleAppearance
    \brief Configuration for the visual properties of particles.
    */
    struct ParticleAppearance
    {
        Vec2 scaleRange = { 0.5f, 1.5f };
        Vec3 startColor = { 1, 1, 1 };
        Vec3 endColor = { 1, 1, 1 };
        bool colorLerp = false; // Interpolate between start and end color

        // Opacity
        bool randomOpacity = false;
        Vec2 opacityRange = { 0.5f, 1.0f };

        // Rotation
        bool rotateParticles = false;
        Vec2 rotationSpeedRange = { -90.0f, 90.0f };

        void Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& allocator) const
        {
            out.SetObject();

            rapidjson::Value scaleRangeVal(rapidjson::kArrayType);
            scaleRangeVal.PushBack(scaleRange.x, allocator).PushBack(scaleRange.y, allocator);
            out.AddMember("scaleRange", scaleRangeVal, allocator);

            rapidjson::Value startColorVal(rapidjson::kArrayType);
            startColorVal.PushBack(startColor.x, allocator).PushBack(startColor.y, allocator).PushBack(startColor.z, allocator);
            out.AddMember("startColor", startColorVal, allocator);

            rapidjson::Value endColorVal(rapidjson::kArrayType);
            endColorVal.PushBack(endColor.x, allocator).PushBack(endColor.y, allocator).PushBack(endColor.z, allocator);
            out.AddMember("endColor", endColorVal, allocator);

            out.AddMember("colorLerp", colorLerp, allocator);
            out.AddMember("randomOpacity", randomOpacity, allocator);

            rapidjson::Value opacityRangeVal(rapidjson::kArrayType);
            opacityRangeVal.PushBack(opacityRange.x, allocator).PushBack(opacityRange.y, allocator);
            out.AddMember("opacityRange", opacityRangeVal, allocator);

            out.AddMember("rotateParticles", rotateParticles, allocator);

            rapidjson::Value rotSpeedVal(rapidjson::kArrayType);
            rotSpeedVal.PushBack(rotationSpeedRange.x, allocator).PushBack(rotationSpeedRange.y, allocator);
            out.AddMember("rotationSpeedRange", rotSpeedVal, allocator);
        }

        void Deserialize(const rapidjson::Value& in)
        {
            if (in.HasMember("scaleRange") && in["scaleRange"].IsArray())
            {
                auto arr = in["scaleRange"].GetArray();
                scaleRange = { arr[0].GetFloat(), arr[1].GetFloat() };
            }
            if (in.HasMember("startColor") && in["startColor"].IsArray())
            {
                auto arr = in["startColor"].GetArray();
                startColor = { arr[0].GetFloat(), arr[1].GetFloat(), arr[2].GetFloat() };
            }
            if (in.HasMember("endColor") && in["endColor"].IsArray())
            {
                auto arr = in["endColor"].GetArray();
                endColor = { arr[0].GetFloat(), arr[1].GetFloat(), arr[2].GetFloat() };
            }
            if (in.HasMember("colorLerp")) colorLerp = in["colorLerp"].GetBool();
            if (in.HasMember("randomOpacity")) randomOpacity = in["randomOpacity"].GetBool();
            if (in.HasMember("opacityRange") && in["opacityRange"].IsArray())
            {
                auto arr = in["opacityRange"].GetArray();
                opacityRange = { arr[0].GetFloat(), arr[1].GetFloat() };
            }
            if (in.HasMember("rotateParticles")) rotateParticles = in["rotateParticles"].GetBool();
            if (in.HasMember("rotationSpeedRange") && in["rotationSpeedRange"].IsArray())
            {
                auto arr = in["rotationSpeedRange"].GetArray();
                rotationSpeedRange = { arr[0].GetFloat(), arr[1].GetFloat() };
            }
        }
    };

    /*
    \struct FadeSettings
    \brief Configuration for alpha fading behavior at the start and end of particle life.
    */
    struct FadeSettings
    {
        // Fade In (when particle spawns)
        bool fadeIn = false;
        float fadeInDuration = 0.3f;

        // Fade Out (before particle dies)
        bool fadeOut = true;
        float fadeOutDuration = 0.5f;

        // ScreenFill stuff
        bool fadeAtEdges = true; // Fade near screen edges
        float edgeFadeDistance = 150.0f; // Distance from edge to start fading

        void Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& allocator) const
        {
            out.SetObject();
            out.AddMember("fadeIn", fadeIn, allocator);
            out.AddMember("fadeInDuration", fadeInDuration, allocator);
            out.AddMember("fadeOut", fadeOut, allocator);
            out.AddMember("fadeOutDuration", fadeOutDuration, allocator);
            out.AddMember("fadeAtEdges", fadeAtEdges, allocator);
            out.AddMember("edgeFadeDistance", edgeFadeDistance, allocator);
        }

        void Deserialize(const rapidjson::Value& in)
        {
            if (in.HasMember("fadeIn")) fadeIn = in["fadeIn"].GetBool();
            if (in.HasMember("fadeInDuration")) fadeInDuration = in["fadeInDuration"].GetFloat();
            if (in.HasMember("fadeOut")) fadeOut = in["fadeOut"].GetBool();
            if (in.HasMember("fadeOutDuration")) fadeOutDuration = in["fadeOutDuration"].GetFloat();
            if (in.HasMember("fadeAtEdges")) fadeAtEdges = in["fadeAtEdges"].GetBool();
            if (in.HasMember("edgeFadeDistance")) edgeFadeDistance = in["edgeFadeDistance"].GetFloat();
        }
    };

    /*
    \struct ParticlePhysics
    \brief Configuration for movement and kinematics.
    */
    struct ParticlePhysics
    {
        Vec2 speedRange = { 50, 100 };
        Vec2 lifetimeRange = { 1.0f, 3.0f }; // Not used for ScreenFill
        Vec2 gravity = { 0, -100 };

        // Drag (air resistance)
        float drag = 0.0f; // 0 = no drag, 1 = heavy drag

        void Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& allocator) const
        {
            out.SetObject();

            rapidjson::Value speedRangeVal(rapidjson::kArrayType);
            speedRangeVal.PushBack(speedRange.x, allocator).PushBack(speedRange.y, allocator);
            out.AddMember("speedRange", speedRangeVal, allocator);

            rapidjson::Value lifetimeRangeVal(rapidjson::kArrayType);
            lifetimeRangeVal.PushBack(lifetimeRange.x, allocator).PushBack(lifetimeRange.y, allocator);
            out.AddMember("lifetimeRange", lifetimeRangeVal, allocator);

            rapidjson::Value gravityVal(rapidjson::kArrayType);
            gravityVal.PushBack(gravity.x, allocator).PushBack(gravity.y, allocator);
            out.AddMember("gravity", gravityVal, allocator);

            out.AddMember("drag", drag, allocator);
        }

        void Deserialize(const rapidjson::Value& in)
        {
            if (in.HasMember("speedRange") && in["speedRange"].IsArray())
            {
                auto arr = in["speedRange"].GetArray();
                speedRange = { arr[0].GetFloat(), arr[1].GetFloat() };
            }
            if (in.HasMember("lifetimeRange") && in["lifetimeRange"].IsArray())
            {
                auto arr = in["lifetimeRange"].GetArray();
                lifetimeRange = { arr[0].GetFloat(), arr[1].GetFloat() };
            }
            if (in.HasMember("gravity") && in["gravity"].IsArray())
            {
                auto arr = in["gravity"].GetArray();
                gravity = { arr[0].GetFloat(), arr[1].GetFloat() };
            }
            if (in.HasMember("drag")) drag = in["drag"].GetFloat();
        }
    };

    /*!
    \struct SpawnSettings
    \brief Configuration for the spatial distribution and direction of spawned particles.
    */
    struct SpawnSettings
    {
        // For Burst/Continuous
        Vec2 spawnOffset = { 0, 0 };     // Offset from emitter position
        float spawnRadius = 0.0f;        // Spawn in circle (0 = single point)
        bool useEmissionCone = false;    // Limit emission direction
        float emissionAngle = 0.0f;      // Center angle (0 = right, 90 = up)
        float emissionSpread = 360.0f;   // Cone width (360 = all directions)

        void Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& allocator) const
        {
            out.SetObject();

            rapidjson::Value offsetVal(rapidjson::kArrayType);
            offsetVal.PushBack(spawnOffset.x, allocator).PushBack(spawnOffset.y, allocator);
            out.AddMember("spawnOffset", offsetVal, allocator);

            out.AddMember("spawnRadius", spawnRadius, allocator);
            out.AddMember("useEmissionCone", useEmissionCone, allocator);
            out.AddMember("emissionAngle", emissionAngle, allocator);
            out.AddMember("emissionSpread", emissionSpread, allocator);
        }

        void Deserialize(const rapidjson::Value& in)
        {
            if (in.HasMember("spawnOffset") && in["spawnOffset"].IsArray())
            {
                auto arr = in["spawnOffset"].GetArray();
                spawnOffset = { arr[0].GetFloat(), arr[1].GetFloat() };
            }
            if (in.HasMember("spawnRadius")) spawnRadius = in["spawnRadius"].GetFloat();
            if (in.HasMember("useEmissionCone")) useEmissionCone = in["useEmissionCone"].GetBool();
            if (in.HasMember("emissionAngle")) emissionAngle = in["emissionAngle"].GetFloat();
            if (in.HasMember("emissionSpread")) emissionSpread = in["emissionSpread"].GetFloat();
        }
    };

    /*
    \struct EmissionSettings
    \brief Configuration for timing and rate of particle generation.
    */
    struct EmissionSettings
    {
        float emissionRate = 50.0f; // Particles per second (for continuous)

        // Burst stuff
        bool loop = false; // Loop burst after delay
        float loopDelay = 1.0f; // Delay between burst loops

        void Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& allocator) const
        {
            out.SetObject();
            out.AddMember("emissionRate", emissionRate, allocator);
            out.AddMember("loop", loop, allocator);
            out.AddMember("loopDelay", loopDelay, allocator);
        }

        void Deserialize(const rapidjson::Value& in)
        {
            if (in.HasMember("emissionRate")) emissionRate = in["emissionRate"].GetFloat();
            if (in.HasMember("loop")) loop = in["loop"].GetBool();
            if (in.HasMember("loopDelay")) loopDelay = in["loopDelay"].GetFloat();
        }
    };

    /*
    \struct ScreenFillSettings
    \brief Specialized settings for EmitterMode::ScreenFill (e.g. Rain/Snow).
    */
    struct ScreenFillSettings
    {
        // Velocity ranges for viewport particles
        Vec2 velocityXRange = { -20, 20 };
        Vec2 velocityYRange = { -150, -80 };

        // Spawn behavior
        bool spawnAtTop = true; // Spawn new particles at top of screen
        float spawnMargin = 100.0f; // How far above/below screen to spawn

        void Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& allocator) const
        {
            out.SetObject();

            rapidjson::Value velXVal(rapidjson::kArrayType);
            velXVal.PushBack(velocityXRange.x, allocator).PushBack(velocityXRange.y, allocator);
            out.AddMember("velocityXRange", velXVal, allocator);

            rapidjson::Value velYVal(rapidjson::kArrayType);
            velYVal.PushBack(velocityYRange.x, allocator).PushBack(velocityYRange.y, allocator);
            out.AddMember("velocityYRange", velYVal, allocator);

            out.AddMember("spawnAtTop", spawnAtTop, allocator);
            out.AddMember("spawnMargin", spawnMargin, allocator);
        }

        void Deserialize(const rapidjson::Value& in)
        {
            if (in.HasMember("velocityXRange") && in["velocityXRange"].IsArray())
            {
                auto arr = in["velocityXRange"].GetArray();
                velocityXRange = { arr[0].GetFloat(), arr[1].GetFloat() };
            }
            if (in.HasMember("velocityYRange") && in["velocityYRange"].IsArray())
            {
                auto arr = in["velocityYRange"].GetArray();
                velocityYRange = { arr[0].GetFloat(), arr[1].GetFloat() };
            }
            if (in.HasMember("spawnAtTop")) spawnAtTop = in["spawnAtTop"].GetBool();
            if (in.HasMember("spawnMargin")) spawnMargin = in["spawnMargin"].GetFloat();
        }
    };

    /*!
    \struct EmitterInstance
    \brief A single, self-contained particle effect instance.
    An entity can hold multiple EmitterInstance to create effects.
    */
    struct EmitterInstance
    {
        std::string name = "Emitter";

        // Core settings
        EmitterMode mode = EmitterMode::Continuous;
        int maxParticles = 100;
        std::string textureName = "whitePixel";
        bool isActive = true; // Enable/disable emitter

        ParticleAppearance appearance;
        FadeSettings fade;
        ParticlePhysics physics;
        SpawnSettings spawn;
        EmissionSettings emission;
        ScreenFillSettings screenFill;

        std::vector<Particle> particles;
        bool initialized = false;
        float emissionTimer = 0.0f;
        float burstTimer = 0.0f;
        Vec2 screenMin = { 0, 0 };
        Vec2 screenMax = { 1920, 1080 };

        // Start/restart the emitter
        void Play()
        {
            isActive = true;
            initialized = false; // Force re-initialization
            emissionTimer = 0.0f;
            burstTimer = 0.0f;
        }

        // Stop spawning new particles
        void Stop()
        {
            isActive = false;
        }

        // Stop and immediately clear all particles
        void StopAndClear()
        {
            isActive = false;
            particles.clear();
            initialized = false;
        }

        // Pause
        void Pause()
        {
            isActive = false;
        }

        // Resume from pause
        void Resume()
        {
            isActive = true;
        }

        // Check if currently emitting particles
        bool IsPlaying() const
        {
            return isActive;
        }

        // Check if any particles are alive
        bool HasActiveParticles() const
        {
            for (const auto& p : particles)
            {
                if (p.active) return true;
            }
            return false;
        }

        // Get count of active particles
        int GetActiveParticleCount() const
        {
            int count = 0;
            for (const auto& p : particles)
            {
                if (p.active) count++;
            }
            return count;
        }

        void Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& allocator) const
        {
            out.SetObject();

            out.AddMember("mode", static_cast<int>(mode), allocator);
            out.AddMember("maxParticles", maxParticles, allocator);
            out.AddMember("isActive", isActive, allocator);

            rapidjson::Value textureVal;
            textureVal.SetString(textureName.c_str(), static_cast<rapidjson::SizeType>(textureName.size()), allocator);
            out.AddMember("textureName", textureVal, allocator);

            rapidjson::Value appearanceVal;
            appearance.Serialize(appearanceVal, allocator);
            out.AddMember("appearance", appearanceVal, allocator);

            rapidjson::Value fadeVal;
            fade.Serialize(fadeVal, allocator);
            out.AddMember("fade", fadeVal, allocator);

            rapidjson::Value physicsVal;
            physics.Serialize(physicsVal, allocator);
            out.AddMember("physics", physicsVal, allocator);

            rapidjson::Value spawnVal;
            spawn.Serialize(spawnVal, allocator);
            out.AddMember("spawn", spawnVal, allocator);

            rapidjson::Value emissionVal;
            emission.Serialize(emissionVal, allocator);
            out.AddMember("emission", emissionVal, allocator);

            rapidjson::Value screenFillVal;
            screenFill.Serialize(screenFillVal, allocator);
            out.AddMember("screenFill", screenFillVal, allocator);
        }

        void Deserialize(const rapidjson::Value& in)
        {
            if (in.HasMember("mode")) mode = static_cast<EmitterMode>(in["mode"].GetInt());
            if (in.HasMember("maxParticles")) maxParticles = in["maxParticles"].GetInt();
            if (in.HasMember("isActive")) isActive = in["isActive"].GetBool();
            if (in.HasMember("textureName")) textureName = in["textureName"].GetString();

            if (in.HasMember("appearance")) appearance.Deserialize(in["appearance"]);
            if (in.HasMember("fade")) fade.Deserialize(in["fade"]);
            if (in.HasMember("physics")) physics.Deserialize(in["physics"]);
            if (in.HasMember("spawn")) spawn.Deserialize(in["spawn"]);
            if (in.HasMember("emission")) emission.Deserialize(in["emission"]);
            if (in.HasMember("screenFill")) screenFill.Deserialize(in["screenFill"]);

            particles.reserve(maxParticles);
        }
    };

    struct ParticleEmitter
    {
        std::vector<EmitterInstance> emitters;

        // Add a new emitter
        int AddEmitter(const std::string& name = "New Emitter")
        {
            EmitterInstance emitter;
            emitter.name = name;
            emitter.particles.reserve(emitter.maxParticles);
            emitters.push_back(emitter);
            return static_cast<int>(emitters.size() - 1);
        }

        // Remove emitter by index
        void RemoveEmitter(int index)
        {
            if (index >= 0 && index < static_cast<int>(emitters.size()))
                emitters.erase(emitters.begin() + index);
        }

        // Get emitter by index
        EmitterInstance* GetEmitter(int index)
        {
            if (index >= 0 && index < static_cast<int>(emitters.size()))
                return &emitters[index];
            return nullptr;
        }

        // Get emitter by name
        EmitterInstance* GetEmitter(const std::string& name)
        {
            for (auto& e : emitters)
                if (e.name == name) return &e;
            return nullptr;
        }

        int GetEmitterCount() const { return static_cast<int>(emitters.size()); }

        void PlayAll()
        {
            for (auto& e : emitters) e.Play();
        }

        void StopAll()
        {
            for (auto& e : emitters) e.Stop();
        }

        void Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& allocator) const
        {
            out.SetObject();
            rapidjson::Value emittersArr(rapidjson::kArrayType);

            for (const auto& emitter : emitters)
            {
                rapidjson::Value emitterVal;
                emitter.Serialize(emitterVal, allocator);
                emittersArr.PushBack(emitterVal, allocator);
            }

            out.AddMember("emitters", emittersArr, allocator);
        }

        void Deserialize(const rapidjson::Value& in)
        {
            if (in.HasMember("emitters") && in["emitters"].IsArray())
            {
                for (const auto& emitterVal : in["emitters"].GetArray())
                {
                    EmitterInstance emitter;
                    emitter.Deserialize(emitterVal);
                    emitters.push_back(emitter);
                }
            }
        }
    };
}