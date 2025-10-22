#pragma once
#include "Math/Math.h"
#include <string>
#include <unordered_map>

namespace Uma_Engine
{
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

    class SpriteAnimator
    {
    public:
        SpriteAnimator()
            : currentFrame(0), playing(true), timer(0.0f) {}

        void AddClip(const std::string& name, const AnimationClip& clip)
        {
            clips[name] = clip;
            if (currentClip.empty())
            {
                currentClip = name;
            }
        }

        void AddClip(const std::string& name, int framesX, int framesY,
            int startFrame, int frameCount, float fps = 10.0f, bool loop = true)
        {
            AnimationClip clip(framesX, framesY, startFrame, frameCount, fps, loop);
            AddClip(name, clip);
        }

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

        void Update(float dt)
        {
            if (!playing || currentClip.empty()) return;

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

        void Reset()
        {
            currentFrame = 0;
            timer = 0.0f;
            playing = true;
        }

        const std::string& GetCurrentClip() const
        {
            return currentClip;
        }

        bool IsPlaying() const
        {
            return playing;
        }

        bool HasFinished() const
        {
            return !playing;
        }

    private:
        std::unordered_map<std::string, AnimationClip> clips;
        std::string currentClip;
        int currentFrame;
        bool playing;
        float timer;
    };
}