/*!
\file    SpriteAnimator.hpp
\par     Project: GAM200
\par     Course: CSD2401
\par     Section A
\par     Software Engineering Project 3

\author  Javier Chua Dong Qing (100%)
\par     E-mail: javierdongqing.chua@digipen.edu
\par     DigiPen login: javierdongqing.chua

\brief
Defines the SpriteAnimator class and AnimationClip struct. This system manages
2D sprite sheet animations, looping and UV coordinate calculation to
render animated sprites

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once
#include "Math/Math.h"
#include <string>
#include <unordered_map>

namespace Uma_Engine
{
    /**
     * \struct AnimationClip
     * \brief Defines a single animation sequence from spritesheet
     */
    struct AnimationClip
    {
        int framesX;        // Frames in X direction (total columns in sheet)
        int framesY;        // Frames in Y direction (total rows in sheet)
        int startFrame;     // Starting frame in sprite sheet
        int frameCount;     // Number of frames in this animation
        float speed;        // Frames per second
        bool loop;          // Loop?

        AnimationClip()
            : framesX(1), framesY(1), startFrame(0), frameCount(1),
            speed(10.0f), loop(true) {}

        AnimationClip(int fx, int fy, int start, int count, float fps = 10.0f, bool shouldLoop = true)
            : framesX(fx), framesY(fy), startFrame(start), frameCount(count),
            speed(fps), loop(shouldLoop) {}
    };

    /**
     * \class SpriteAnimator
     * \brief Manages multiple AnimationClips and controls playback state
     */
    class SpriteAnimator
    {
    public:
        SpriteAnimator()
            : currentFrame(0), playing(true), timer(0.0f) {}

        /**
         * \brief Adds or overwrites an animation clip
         * \param name Unique identifier for the clip
         * \param clip AnimationClip object to add
         */
        void AddClip(const std::string& name, const AnimationClip& clip)
        {
            clips[name] = clip;
            if (currentClip.empty())
            {
                currentClip = name;
            }
        }

        /**
         * \brief Overload to add a clip by constructing it
         * \param name Unique identifier for the clip
         * \param framesX Total columns in the sprite sheet
         * \param framesY Total rows in the sprite sheet
         * \param startFrame Index of the first frame
         * \param frameCount Total number of frames in clip
         * \param fps Frames per second
         * \param loop Whether the animation should loop
         */
        void AddClip(const std::string& name, int framesX, int framesY,
            int startFrame, int frameCount, float fps = 10.0f, bool loop = true)
        {
            AnimationClip clip(framesX, framesY, startFrame, frameCount, fps, loop);
            AddClip(name, clip);
        }

        /**
         * \brief Plays a specified animation clip
         * \param name Name of the clip to play
         * \param restart If true, the animation will restart from frame 0
         */
        void Play(const std::string& name, bool restart = false)
        {
            if (clips.find(name) == clips.end())
                return;

            if (currentClip != name || restart)
            {
                currentClip = name;
                currentFrame = 0;
                timer = 0.0f;
                playing = true;
            }
        }

        /**
         * \brief Updates the current frame and animation timer
         * \param dt Delta time
         */
        void Update(float dt)
        {
            if (currentClip.empty()) return;

            if (!playing)
            {
                currentFrame = 0;
            }
            else
            {
                auto it = clips.find(currentClip);
                if (it == clips.end()) return;

                const AnimationClip& clip = it->second;

                timer += dt;
                float frameTime = 1.0f / clip.speed;

                while (timer >= frameTime)
                {
                    timer -= frameTime;
                    currentFrame++;

                    if (currentFrame >= clip.frameCount)
                    {
                        if (clip.loop)
                            currentFrame = 0;
                        else
                        {
                            currentFrame = clip.frameCount - 1;
                            playing = false;
                        }
                    }
                }
            }
        }

        /**
         * \brief Calculates the UV coordinates for the current animation frame
         * \param uvOffset Calculated UV offset (top-left corner of the frame)
         * \param uvSize Calculated UV size (dimensions of a single frame)
         */
        void GetUVs(Vec2& uvOffset, Vec2& uvSize) const
        {
            auto it = clips.find(currentClip);
            if (it == clips.end())
            {
                uvOffset = Vec2(0.0f, 0.0f);
                uvSize = Vec2(1.0f, 1.0f);
                return;
            }

            const AnimationClip& clip = it->second;

            float frameWidth = 1.0f / clip.framesX;
            float frameHeight = 1.0f / clip.framesY;

            // Calculate absolute frame index in sprite sheet
            int absoluteFrame = clip.startFrame + currentFrame;

            int col = absoluteFrame % clip.framesX;
            int row = absoluteFrame / clip.framesX;

            uvOffset.x = col * frameWidth;
            uvOffset.y = row * frameHeight;
            uvSize.x = frameWidth;
            uvSize.y = frameHeight;
        }

        /**
         * \brief Resets the current clip to its first frame and restarts playback
         */
        void Reset()
        {
            currentFrame = 0;
            timer = 0.0f;
            playing = true;
        }

        /**
         * \brief Gets name of the currently playing animation clip
         * \return Const reference to the current clip's name string
         */
        const std::string& GetCurrentClip() const
        {
            return currentClip;
        }

        /**
         * \brief Checks if the animation is currently playing
         * \return true if playing
         */
        bool IsPlaying() const
        {
            return playing;
        }

        /**
         * \brief Checks if a non-looping animation has finished
         * \return true if the animation is not playing
         */
        bool HasFinished() const
        {
            return !playing;
        }

        /**
         * \brief Gets read-only reference to the internal clip map
         * \return Const reference to the clip map
         */
        const std::unordered_map<std::string, AnimationClip>& GetClips() const
        {
            return clips;
        }

    private:
        std::unordered_map<std::string, AnimationClip> clips; // Map storing all animation clips by name
        std::string currentClip;                              // Name of current active clip
        int currentFrame;                                     // Current frame index
        bool playing;                                         // If currently playing
        float timer;                                          // Accumulate delta time to control frame rate
    };
}