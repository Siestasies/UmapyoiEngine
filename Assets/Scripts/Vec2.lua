--! @file Vec2.lua
--! @brief Lua-side Vec2 implementation that mirrors the C++ registered type
--! @details Use this for pure Lua calculations or when C++ Vec2 isn't available

--! @class Vec2
--! @brief A 2D vector class with mathematical operations and utilities
local Vec2 = {}
Vec2.__index = Vec2


--! @brief Constructor for creating a new Vec2 instance
--! @param x number X coordinate (default: 0)
--! @param y number Y coordinate (default: 0)
--! @return Vec2 A new Vec2 object
function Vec2.new(x, y)
    local self = setmetatable({}, Vec2)
    self.x = x or 0
    self.y = y or 0
    return self
end


--! @brief Addition metamethod
--! @param a Vec2 First vector
--! @param b Vec2 Second vector
--! @return Vec2 The sum of a and b
function Vec2.__add(a, b)
    return Vec2.new(a.x + b.x, a.y + b.y)
end


--! @brief Subtraction metamethod
--! @param a Vec2 First vector
--! @param b Vec2 Second vector
--! @return Vec2 The difference of a and b
function Vec2.__sub(a, b)
    return Vec2.new(a.x - b.x, a.y - b.y)
end


--! @brief Multiplication metamethod
--! @details Supports scalar multiplication and dot product
--! @param a Vec2|number First operand
--! @param b Vec2|number Second operand
--! @return Vec2|number Result (Vec2 for scalar mult, number for dot product)
function Vec2.__mul(a, b)
    if type(a) == "number" then
        return Vec2.new(a * b.x, a * b.y)
    elseif type(b) == "number" then
        return Vec2.new(a.x * b, a.y * b)
    else
        return a.x * b.x + a.y * b.y
    end
end


--! @brief Division metamethod
--! @param a Vec2 The vector to divide
--! @param b number The scalar divisor
--! @return Vec2 The scaled vector
--! @throws error If b is not a number
function Vec2.__div(a, b)
    if type(b) == "number" then
        return Vec2.new(a.x / b, a.y / b)
    else
        error("Vec2 can only be divided by a scalar")
    end
end


--! @brief Unary negation metamethod
--! @param a Vec2 The vector to negate
--! @return Vec2 The negated vector
function Vec2.__unm(a)
    return Vec2.new(-a.x, -a.y)
end


--! @brief Equality metamethod
--! @param a Vec2 First vector
--! @param b Vec2 Second vector
--! @return boolean True if both vectors are equal
function Vec2.__eq(a, b)
    return a.x == b.x and a.y == b.y
end


--! @brief String representation metamethod
--! @param a Vec2 The vector to convert
--! @return string String representation in format "Vec2(x, y)"
function Vec2.__tostring(a)
    return "Vec2(" .. a.x .. ", " .. a.y .. ")"
end


--! @brief Calculate the length (magnitude) of the vector
--! @return number The vector length
function Vec2:length()
    return math.sqrt(self.x * self.x + self.y * self.y)
end


--! @brief Calculate the squared length (avoids sqrt for comparisons)
--! @return number The squared length
function Vec2:lengthSquared()
    return self.x * self.x + self.y * self.y
end


--! @brief Normalize the vector to unit length
--! @return Vec2 A new normalized vector
function Vec2:normalize()
    local len = self:length()
    if len > 0 then
        return Vec2.new(self.x / len, self.y / len)
    end
    return Vec2.new(0, 0)
end


--! @brief Alias for normalize()
--! @return Vec2 A new normalized vector
function Vec2:normalized()
    return self:normalize()
end


--! @brief Calculate the dot product with another vector
--! @param other Vec2 The other vector
--! @return number The dot product
function Vec2:dot(other)
    return self.x * other.x + self.y * other.y
end


--! @brief Calculate the distance to another vector
--! @param other Vec2 The target vector
--! @return number The distance
function Vec2:distance(other)
    local dx = self.x - other.x
    local dy = self.y - other.y
    return math.sqrt(dx * dx + dy * dy)
end


--! @brief Calculate the squared distance to another vector
--! @param other Vec2 The target vector
--! @return number The squared distance
function Vec2:distanceSquared(other)
    local dx = self.x - other.x
    local dy = self.y - other.y
    return dx * dx + dy * dy
end


--! @brief Linear interpolation between this vector and another
--! @param other Vec2 The target vector
--! @param t number Interpolation factor (0 = this, 1 = other)
--! @return Vec2 The interpolated vector
function Vec2:lerp(other, t)
    return Vec2.new(
        self.x + (other.x - self.x) * t,
        self.y + (other.y - self.y) * t
    )
end


--! @brief Create a copy of this vector
--! @return Vec2 A new independent copy
function Vec2:clone()
    return Vec2.new(self.x, self.y)
end


--! @brief Create a zero vector (0, 0)
--! @return Vec2 A new zero vector
function Vec2.zero()
    return Vec2.new(0, 0)
end


--! @brief Create a unit vector (1, 1)
--! @return Vec2 A new unit vector
function Vec2.one()
    return Vec2.new(1, 1)
end


--! @brief Create an up direction vector (0, 1)
--! @return Vec2 A new up vector
function Vec2.up()
    return Vec2.new(0, 1)
end


--! @brief Create a down direction vector (0, -1)
--! @return Vec2 A new down vector
function Vec2.down()
    return Vec2.new(0, -1)
end


--! @brief Create a left direction vector (-1, 0)
--! @return Vec2 A new left vector
function Vec2.left()
    return Vec2.new(-1, 0)
end


--! @brief Create a right direction vector (1, 0)
--! @return Vec2 A new right vector
function Vec2.right()
    return Vec2.new(1, 0)
end


return Vec2
