local RNG = {
    currentSeed = nil,
}
RNG.__index = RNG

-- seed the RNG system
function RNG.seed(seed)
    RNG.currentSeed = seed or 1
end

function RNG.new()
    local self = setmetatable({}, RNG)

    if RNG.currentSeed == nil then
        RNG.seed(1)
    end

    self.seed = RNG.currentSeed
    self.state = self.seed

    RNG.currentSeed = RNG.currentSeed + 1
    return self
end

-- core xorshift32 algorithm
function RNG:nextInt()
    local x = self.state

    x = x ~ (x << 13)
    x = x ~ (x >> 17)
    x = x ~ (x << 5)

    self.state = x & 0xffffffff
    return self.state
end

-- random float (0–1)
function RNG:random()
    return self:nextInt() / 0xffffffff
end

-- random integer range
function RNG:randomRange(min, max)
    return min + math.floor(self:random() * (max - min + 1))
end

return RNG