#pragma once

#include "Engine/Math/Math.h"

struct Colour
{
    float r, g, b, a;
    Color(float r = 1.f, float g = 1.f, float b = 1.f, float a = 1.f) : r(r), g(g), b(b), a(a) {}
};

struct Rect
{
    float x, y, w, h;
    Rect(float x = 0.f, float y = 0.f, float w = 0.f, float h = 0.f) : x(x), y(y), w(w), h(h) {}

    bool Contains(Vec2 point) const
    {
        return point.x >= x && point.x <= x + w && point.y >= y && point.y <= y + h;
    }
};

enum class Anchor
{
    TopLeft, TopCentre, TopRight,
    MiddleLeft, MiddleCentre, MiddleRight,
    BottomLeft, BottomCentre, BottomRight,
    StretchAll
};