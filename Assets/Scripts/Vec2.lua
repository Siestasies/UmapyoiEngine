-- Vec2.lua
-- Lua-side Vec2 implementation that mirrors the C++ registered type
-- Use this for pure Lua calculations or when C++ Vec2 isn't available

local Vec2 = {}
Vec2.__index = Vec2

-- Constructor
function Vec2.new(x, y)
    local self = setmetatable({}, Vec2)
    self.x = x or 0
    self.y = y or 0
    return self
end

-- Metamethods for operators
function Vec2.__add(a, b)
    return Vec2.new(a.x + b.x, a.y + b.y)
end

function Vec2.__sub(a, b)
    return Vec2.new(a.x - b.x, a.y - b.y)
end

function Vec2.__mul(a, b)
    if type(a) == "number" then
        return Vec2.new(a * b.x, a * b.y)
    elseif type(b) == "number" then
        return Vec2.new(a.x * b, a.y * b)
    else
        -- Dot product
        return a.x * b.x + a.y * b.y
    end
end

function Vec2.__div(a, b)
    if type(b) == "number" then
        return Vec2.new(a.x / b, a.y / b)
    else
        error("Vec2 can only be divided by a scalar")
    end
end

function Vec2.__unm(a)
    return Vec2.new(-a.x, -a.y)
end

function Vec2.__eq(a, b)
    return a.x == b.x and a.y == b.y
end

function Vec2.__tostring(a)
    return "Vec2(" .. a.x .. ", " .. a.y .. ")"
end

-- Utility methods
function Vec2:length()
    return math.sqrt(self.x * self.x + self.y * self.y)
end

function Vec2:lengthSquared()
    return self.x * self.x + self.y * self.y
end

function Vec2:normalize()
    local len = self:length()
    if len > 0 then
        return Vec2.new(self.x / len, self.y / len)
    end
    return Vec2.new(0, 0)
end

function Vec2:normalized()
    return self:normalize()
end

function Vec2:dot(other)
    return self.x * other.x + self.y * other.y
end

function Vec2:distance(other)
    local dx = self.x - other.x
    local dy = self.y - other.y
    return math.sqrt(dx * dx + dy * dy)
end

function Vec2:distanceSquared(other)
    local dx = self.x - other.x
    local dy = self.y - other.y
    return dx * dx + dy * dy
end

function Vec2:lerp(other, t)
    return Vec2.new(
        self.x + (other.x - self.x) * t,
        self.y + (other.y - self.y) * t
    )
end

function Vec2:clone()
    return Vec2.new(self.x, self.y)
end

-- Static methods
function Vec2.zero()
    return Vec2.new(0, 0)
end

function Vec2.one()
    return Vec2.new(1, 1)
end

function Vec2.up()
    return Vec2.new(0, 1)
end

function Vec2.down()
    return Vec2.new(0, -1)
end

function Vec2.left()
    return Vec2.new(-1, 0)
end

function Vec2.right()
    return Vec2.new(1, 0)
end

return Vec2