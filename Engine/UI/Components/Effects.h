/*!
\file   Effects.h
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A
\par    Software Engineering Project 4

\author Jedrek Lee Jing Wei (100%)
\par    E-mail: jedrekjingwei.lee@digipen.edu
\par    DigiPen login: jedrekjingwei.lee

\brief
Defines the EffectClip and Effects UI components for property-based UI animations.

EffectClip represents a single tween that interpolates a UI property (position,
color, opacity, spritesheet frame) over time with configurable easing, looping,
and ping-pong playback. Effects aggregates multiple clips with play/stop/reset
controls and JSON serialization.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "../Core/UITypes.h"
#include "../../Math/Math.h"
#include "rapidjson/document.h"
#include <vector>
#include <functional>

namespace Uma_UI
{
    /*!
     * \class EffectClip
     * \brief A single animation clip that tweens a UI property over time.
     */
    class EffectClip
    {
    public:
        std::string name;
        EffectProperty property = EffectProperty::Position;
        EasingType easing = EasingType::Linear;

        float duration = 1.0f;
        float delay = 0.0f;
        bool loop = false;
        bool applyToChildren = false;

        // Start/end values (interpretation depends on property)
        Vec2 startVec2 = Vec2(0.0f, 0.0f);
        Vec2 endVec2 = Vec2(0.0f, 0.0f);

        Color startColor = Color::White();
        Color endColor = Color::White();

        float startFloat = 0.0f;
        float endFloat = 1.0f;

        // Spritesheet animation (used when property == SpritesheetFrame)
        // Frames are specified as flat row-major indices into the spritesheet grid.
        int   startFrame = 0;      // First frame index (inclusive)
        int   endFrame = 0;        // Last frame index (inclusive)
        float fps = 12.0f;         // Playback speed in frames per second
        bool  pingPong = false;    // If true, plays forward then in reverse

        // Cinematic animation (used when property == CinematicFrame)
        // Each entry is an asset path to a full-resolution texture; no spritesheet required.
        // Storage is O(frames) paths rather than one giant atlas, which is optimal for
        // native-resolution images that cannot be packed into a single spritesheet.
        std::vector<std::string> cinematicFrames;  // Ordered list of texture paths
        float cinematicFps = 24.0f;                // Playback speed (frames per second)
        bool  cinematicPingPong = false;            // Play forward then reverse each cycle

        // Runtime state
        float currentTime = 0.0f;
        bool isPlaying = false;
        bool hasStarted = false;

        /*!
         * \brief Resets clip runtime state to initial values.
         */
        void Reset()
        {
            currentTime = 0.0f;
            isPlaying = false;
            hasStarted = false;
        }

        /*!
         * \brief Starts or resumes playback of this clip.
         */
        void Play()
        {
            isPlaying = true;
        }

        /*!
         * \brief Pauses playback without resetting the current time.
         */
        void Pause()
        {
            isPlaying = false;
        }

        /*!
         * \brief Stops playback and resets to the beginning.
         */
        void Stop()
        {
            Reset();
        }

        /*!
         * \brief Computes the normalized progress of the clip.
         * \return Progress value clamped to [0,1].
         */
        float GetProgress() const
        {
            if (duration <= 0.0f) return 1.0f;

            float t = (currentTime - delay) / duration;
            return (std::max)(0.0f, (std::min)(1.0f, t));
        }

        /*!
         * \brief Computes the current spritesheet frame index for SpritesheetFrame effects.
         *
         * Uses fps and currentTime to step through [startFrame, endFrame] in row-major order.
         * Supports looping and ping-pong playback.
         * \return Flat frame index into the spritesheet grid.
         */
        int GetCurrentFrame() const
        {
            int frameCount = endFrame - startFrame + 1;
            if (frameCount <= 0) return startFrame;

            float elapsed = currentTime - delay;
            if (elapsed < 0.0f) return startFrame;

            int totalSteps = static_cast<int>(elapsed * fps);

            if (pingPong)
            {
                int cycleLen = (frameCount > 1) ? (frameCount * 2 - 2) : 1;
                int step = loop ? (totalSteps % cycleLen) : (std::min)(totalSteps, cycleLen - 1);
                int frame = (step < frameCount) ? step : (cycleLen - step);
                return startFrame + frame;
            }
            else
            {
                int step = loop ? (totalSteps % frameCount) : (std::min)(totalSteps, frameCount - 1);
                return startFrame + step;
            }
        }

        /*!
         * \brief Computes the current frame index into cinematicFrames for CinematicFrame effects.
         *
         * Uses cinematicFps and currentTime to step through the frame list.
         * Supports looping and ping-pong playback. When cinematicFrames is empty returns 0.
         * \return Zero-based index into cinematicFrames.
         */
        int GetCurrentCinematicFrame() const
        {
            int frameCount = static_cast<int>(cinematicFrames.size());
            if (frameCount <= 0) return 0;

            float elapsed = currentTime - delay;
            if (elapsed < 0.0f) return 0;

            int totalSteps = static_cast<int>(elapsed * cinematicFps);

            if (cinematicPingPong)
            {
                int cycleLen = (frameCount > 1) ? (frameCount * 2 - 2) : 1;
                int step = loop ? (totalSteps % cycleLen) : (std::min)(totalSteps, cycleLen - 1);
                return (step < frameCount) ? step : (cycleLen - step);
            }
            else
            {
                int step = loop ? (totalSteps % frameCount) : (std::min)(totalSteps, frameCount - 1);
                return step;
            }
        }

        /*!
         * \brief Returns the texture path for the current cinematic frame.
         * \return Asset path string, or empty string if cinematicFrames is empty.
         */
        const std::string& GetCurrentCinematicPath() const
        {
            static const std::string empty;
            if (cinematicFrames.empty()) return empty;
            return cinematicFrames[GetCurrentCinematicFrame()];
        }

        /*!
         * \brief Returns the natural duration of the cinematic clip based on frame count and fps.
         *
         * Useful as a default value when constructing a clip: set duration = GetCinematicDuration().
         * \return Duration in seconds, or 0 if no frames or fps is zero.
         */
        float GetCinematicDuration() const
        {
            if (cinematicFps <= 0.0f || cinematicFrames.empty()) return 0.0f;
            return static_cast<float>(cinematicFrames.size()) / cinematicFps;
        }

        /*!
         * \brief Checks whether the clip has finished playing.
         * \return True if playback has completed past delay plus duration.
         */
        bool IsComplete() const
        {
            if (!hasStarted) return false;
            return currentTime >= (delay + duration);
        }
    };

    /*!
     * \class Effects
     * \brief UI component holding a collection of EffectClip animations.
     */
    class Effects
    {
    public:
        std::vector<EffectClip> clips;
        bool playOnEnable = false;

        // Runtime-only: tracks last-known hierarchy-active state so playOnEnable
        // fires exactly once on each inactive->active transition, never every frame.
        // Never serialized.
        bool _wasActiveInHierarchy = false;

        /*!
         * \brief Adds an effect clip to the collection.
         * \param clip The clip to add.
         */
        void AddClip(const EffectClip& clip)
        {
            clips.push_back(clip);
        }

        /*!
         * \brief Starts playback of all clips.
         */
        void PlayAll()
        {
            for (auto& clip : clips)
            {
                clip.Play();
            }
        }

        /*!
         * \brief Stops and resets all clips.
         */
        void StopAll()
        {
            for (auto& clip : clips)
            {
                clip.Stop();
            }
        }

        /*!
         * \brief Resets all clips to their initial state.
         */
        void ResetAll()
        {
            for (auto& clip : clips)
            {
                clip.Reset();
            }
        }

        /*!
         * \brief Starts playback of a clip at the given index.
         * \param index Zero-based index of the clip.
         */
        void PlayClip(size_t index)
        {
            if (index < clips.size())
            {
                clips[index].Play();
            }
        }

        /*!
         * \brief Pauses a clip at the given index.
         * \param index Zero-based index of the clip.
         */
        void PauseClip(size_t index)
        {
            if (index < clips.size())
            {
                clips[index].Pause();
            }
        }

        /*!
         * \brief Stops and resets a clip at the given index.
         * \param index Zero-based index of the clip.
         */
        void StopClip(size_t index)
        {
            if (index < clips.size())
            {
                clips[index].Stop();
            }
        }

        /*!
         * \brief Resets a clip at the given index to its initial state.
         * \param index Zero-based index of the clip.
         */
        void ResetClip(size_t index)
        {
            if (index < clips.size())
            {
                clips[index].Reset();
            }
        }

        /*!
         * \brief Queries whether a clip at the given index is currently playing.
         * \param index Zero-based index of the clip.
         * \return True if the clip is playing, false if paused, stopped, or index is out of range.
         */
        bool IsClipPlaying(size_t index) const
        {
            if (index < clips.size())
            {
                return clips[index].isPlaying;
            }
            return false;
        }

        /*!
         * \brief Queries whether a clip at the given index has completed.
         * \param index Zero-based index of the clip.
         * \return True if the clip has finished, false otherwise or if index is out of range.
         */
        bool IsClipComplete(size_t index) const
        {
            if (index < clips.size())
            {
                return clips[index].IsComplete();
            }
            return false;
        }

        /*!
         * \brief Returns the number of clips in this Effects component.
         * \return Number of clips.
         */
        size_t GetClipCount() const
        {
            return clips.size();
        }

        /*!
         * \brief Starts playback of all clips matching the given name.
         * \param name Name of the clip(s) to play.
         */
        void PlayClipByName(const std::string& name)
        {
            for (auto& clip : clips)
            {
                if (clip.name == name)
                {
                    clip.Play();
                }
            }
        }

        /*!
         * \brief Pauses all clips matching the given name.
         * \param name Name of the clip(s) to pause.
         */
        void PauseClipByName(const std::string& name)
        {
            for (auto& clip : clips)
            {
                if (clip.name == name)
                {
                    clip.Pause();
                }
            }
        }

        /*!
         * \brief Stops and resets all clips matching the given name.
         * \param name Name of the clip(s) to stop.
         */
        void StopClipByName(const std::string& name)
        {
            for (auto& clip : clips)
            {
                if (clip.name == name)
                {
                    clip.Stop();
                }
            }
        }

        /*!
         * \brief Resets all clips matching the given name to their initial state.
         * \param name Name of the clip(s) to reset.
         */
        void ResetClipByName(const std::string& name)
        {
            for (auto& clip : clips)
            {
                if (clip.name == name)
                {
                    clip.Reset();
                }
            }
        }

        /*!
         * \brief Finds the index of the first clip matching the given name.
         * \param name Name of the clip to find.
         * \return Zero-based index of the clip, or -1 if not found.
         */
        int FindClipIndexByName(const std::string& name) const
        {
            for (size_t i = 0; i < clips.size(); ++i)
            {
                if (clips[i].name == name)
                {
                    return static_cast<int>(i);
                }
            }
            return -1;  // Not found
        }

        /*!
         * \brief Serializes all clips and settings to a JSON value.
         * \param jsonValue Output JSON value to populate.
         * \param allocator RapidJSON allocator for memory management.
         */
        void Serialize(rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator) const
        {
            jsonValue.SetObject();
            jsonValue.AddMember("playOnEnable", playOnEnable, allocator);

            rapidjson::Value clipsArray(rapidjson::kArrayType);
            for (const auto& clip : clips)
            {
                rapidjson::Value clipObj(rapidjson::kObjectType);

                // Serialize name
                rapidjson::Value nameValue;
                nameValue.SetString(clip.name.c_str(), static_cast<rapidjson::SizeType>(clip.name.length()), allocator);
                clipObj.AddMember("name", nameValue, allocator);

                clipObj.AddMember("property", static_cast<int>(clip.property), allocator);
                clipObj.AddMember("easing", static_cast<int>(clip.easing), allocator);
                clipObj.AddMember("duration", clip.duration, allocator);
                clipObj.AddMember("delay", clip.delay, allocator);
                clipObj.AddMember("loop", clip.loop, allocator);
                clipObj.AddMember("applyToChildren", clip.applyToChildren, allocator);

                rapidjson::Value startVec(rapidjson::kObjectType);
                startVec.AddMember("x", clip.startVec2.x, allocator);
                startVec.AddMember("y", clip.startVec2.y, allocator);
                clipObj.AddMember("startVec2", startVec, allocator);

                rapidjson::Value endVec(rapidjson::kObjectType);
                endVec.AddMember("x", clip.endVec2.x, allocator);
                endVec.AddMember("y", clip.endVec2.y, allocator);
                clipObj.AddMember("endVec2", endVec, allocator);

                rapidjson::Value startCol(rapidjson::kObjectType);
                startCol.AddMember("r", clip.startColor.r, allocator);
                startCol.AddMember("g", clip.startColor.g, allocator);
                startCol.AddMember("b", clip.startColor.b, allocator);
                startCol.AddMember("a", clip.startColor.a, allocator);
                clipObj.AddMember("startColor", startCol, allocator);

                rapidjson::Value endCol(rapidjson::kObjectType);
                endCol.AddMember("r", clip.endColor.r, allocator);
                endCol.AddMember("g", clip.endColor.g, allocator);
                endCol.AddMember("b", clip.endColor.b, allocator);
                endCol.AddMember("a", clip.endColor.a, allocator);
                clipObj.AddMember("endColor", endCol, allocator);

                clipObj.AddMember("startFloat", clip.startFloat, allocator);
                clipObj.AddMember("endFloat", clip.endFloat, allocator);

                clipObj.AddMember("startFrame", clip.startFrame, allocator);
                clipObj.AddMember("endFrame", clip.endFrame, allocator);
                clipObj.AddMember("fps", clip.fps, allocator);
                clipObj.AddMember("pingPong", clip.pingPong, allocator);

                // Cinematic frames: store as a JSON array of path strings.
                // This is intentionally separate from the spritesheet fields — each path
                // points to a full-resolution texture loaded individually at runtime.
                clipObj.AddMember("cinematicFps", clip.cinematicFps, allocator);
                clipObj.AddMember("cinematicPingPong", clip.cinematicPingPong, allocator);
                rapidjson::Value pathsArray(rapidjson::kArrayType);
                for (const auto& path : clip.cinematicFrames)
                {
                    rapidjson::Value pathVal;
                    pathVal.SetString(path.c_str(), static_cast<rapidjson::SizeType>(path.length()), allocator);
                    pathsArray.PushBack(pathVal, allocator);
                }
                clipObj.AddMember("cinematicFrames", pathsArray, allocator);

                clipsArray.PushBack(clipObj, allocator);
            }
            jsonValue.AddMember("clips", clipsArray, allocator);
        }

        /*!
         * \brief Deserializes all clips and settings from a JSON value.
         * \param jsonValue Input JSON value to read from.
         */
        void Deserialize(const rapidjson::Value& jsonValue)
        {
            playOnEnable = jsonValue["playOnEnable"].GetBool();

            clips.clear();
            const auto& clipsArray = jsonValue["clips"];
            for (rapidjson::SizeType i = 0; i < clipsArray.Size(); ++i)
            {
                const auto& clipObj = clipsArray[i];

                EffectClip clip;
                clip.name = clipObj.HasMember("name") ? clipObj["name"].GetString() : "";
                clip.property = static_cast<EffectProperty>(clipObj["property"].GetInt());
                clip.easing = static_cast<EasingType>(clipObj["easing"].GetInt());
                clip.duration = clipObj["duration"].GetFloat();
                clip.delay = clipObj["delay"].GetFloat();
                clip.loop = clipObj["loop"].GetBool();
                clip.applyToChildren = clipObj.HasMember("applyToChildren") ? clipObj["applyToChildren"].GetBool() : false;

                const auto& startVec = clipObj["startVec2"];
                clip.startVec2.x = startVec["x"].GetFloat();
                clip.startVec2.y = startVec["y"].GetFloat();

                const auto& endVec = clipObj["endVec2"];
                clip.endVec2.x = endVec["x"].GetFloat();
                clip.endVec2.y = endVec["y"].GetFloat();

                const auto& startCol = clipObj["startColor"];
                clip.startColor.r = startCol["r"].GetFloat();
                clip.startColor.g = startCol["g"].GetFloat();
                clip.startColor.b = startCol["b"].GetFloat();
                clip.startColor.a = startCol["a"].GetFloat();

                const auto& endCol = clipObj["endColor"];
                clip.endColor.r = endCol["r"].GetFloat();
                clip.endColor.g = endCol["g"].GetFloat();
                clip.endColor.b = endCol["b"].GetFloat();
                clip.endColor.a = endCol["a"].GetFloat();

                clip.startFloat = clipObj["startFloat"].GetFloat();
                clip.endFloat = clipObj["endFloat"].GetFloat();

                clip.startFrame = clipObj.HasMember("startFrame") ? clipObj["startFrame"].GetInt() : 0;
                clip.endFrame = clipObj.HasMember("endFrame") ? clipObj["endFrame"].GetInt() : 0;
                clip.fps = clipObj.HasMember("fps") ? clipObj["fps"].GetFloat() : 12.0f;
                clip.pingPong = clipObj.HasMember("pingPong") ? clipObj["pingPong"].GetBool() : false;

                clip.cinematicFps = clipObj.HasMember("cinematicFps") ? clipObj["cinematicFps"].GetFloat() : 24.0f;
                clip.cinematicPingPong = clipObj.HasMember("cinematicPingPong") ? clipObj["cinematicPingPong"].GetBool() : false;
                clip.cinematicFrames.clear();
                if (clipObj.HasMember("cinematicFrames") && clipObj["cinematicFrames"].IsArray())
                {
                    for (const auto& pathVal : clipObj["cinematicFrames"].GetArray())
                    {
                        if (pathVal.IsString())
                        {
                            clip.cinematicFrames.push_back(pathVal.GetString());
                        }
                    }
                }

                clips.push_back(clip);
            }
        }
    };
}