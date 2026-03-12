-- =============================================================================
--  feedback.lua
--  Attach to: feedback cnv  (child of the "feedback" empty under game_manager)
--
--  Hierarchy expected in prefab:
--    game_manager
--      feedback                 <-- THIS SCRIPT LIVES HERE
--        feedback txt 1         (Text + RectTransform)
--        feedback txt 2         (Text + RectTransform)
--        ...                      (up to POOL_SIZE children, all pre-built)
--
--  Each pool-child must have:
--    · Text        – content/colour/fontSize/visible
--    · RectTransform – anchorMin/Max (0.5,0.5), pivot (0.5,0.5),
--                      anchoredPosition (-9999,-9999), sizeDelta (200,80)
--
-- =============================================================================

-- ─────────────────────────────────────────────────────────────────────────────
--  CONFIG  (mirrors FeedbackConfig constants)
-- ─────────────────────────────────────────────────────────────────────────────
local POOL_SIZE          = 32
local LIFETIME           = 1.2      -- seconds
local RISE_SPEED         = 80.0     -- pixels/sec upward
local FALL_SPEED         = 60.0     -- pixels/sec downward (ManaSpend / Warning)
local SPREAD_RADIUS      = 30.0     -- max horizontal jitter in pixels
local FADE_START         = 0.55     -- fraction of lifetime when fade begins
local PUNCH_DURATION     = 0.12     -- seconds for scale pop
local PUNCH_PEAK         = 1.35     -- max scale multiplier during punch

local FONT_SIZE_NORMAL   = 2.0
local FONT_SIZE_CRIT     = 3.0
local FONT_SIZE_AFFINITY = 3.0
local FONT_SIZE_MANA     = 1.0
local FONT_SIZE_WARNING  = 1.0

-- RGBA colour tables
local COLOR = {
    Normal    = { r=1.00, g=1.00, b=1.00 },
    Affinity  = { r=1.00, g=0.84, b=0.00 },
    Critical  = { r=1.00, g=0.30, b=0.10 },
    Heal      = { r=0.30, g=1.00, b=0.45 },
    PlayerHit = { r=1.00, g=0.20, b=0.20 },
    ManaSpend = { r=0.40, g=0.60, b=1.00 },
    ManaGain  = { r=0.55, g=0.85, b=1.00 },
    Warning   = { r=1.00, g=0.65, b=0.10 },
}

local FONT_PATH = "Assets/Fonts/Fujimaru-Regular.ttf"

-- ─────────────────────────────────────────────────────────────────────────────
--  POOL STATE
-- ─────────────────────────────────────────────────────────────────────────────
-- pool[i] = {
--   entity      = Entity,
--   alive       = bool,
--   elapsed     = float,
--   lifetime    = float,
--   basePixX    = float,   -- pixel-space spawn X
--   basePixY    = float,   -- pixel-space spawn Y
--   jitterPixX  = float,
--   baseFontSize= float,
--   fall        = bool,    -- true → falls, false → rises
-- }
local pool = {}

-- xorshift32 state
local randState = 0x9E3779B9

-- ─────────────────────────────────────────────────────────────────────────────
--  HELPERS
-- ─────────────────────────────────────────────────────────────────────────────
local function NextJitter()
    -- xorshift32 returning [0, 1]
    randState = randState ~ (randState << 13)
    randState = randState ~ (randState >> 17)
    randState = randState ~ (randState << 5)
    randState = randState & 0xFFFFFFFF          -- keep 32-bit
    return (randState & 0xFFFF) / 65535.0
end

local function HideSlot(slot)
    slot.alive   = false
    slot.elapsed = 0.0
    SetActiveEntity(slot.entity, false)

    local rt = GetRectTransformFrom(slot.entity)
    if rt then
        rt.anchoredPosition.x = -9999.0
        rt.anchoredPosition.y = -9999.0
        rt.isDirty = true
    end

    local txt = GetTextFrom(slot.entity)
    if txt then
        txt.color.a = 0.0
        txt.visible = false
        txt.text    = ""
    end
end

local function AcquireSlot()
    -- First-pass: find any idle slot
    for i = 1, POOL_SIZE do
        if not pool[i].alive then return i end
    end
    -- Evict furthest-elapsed (nearest to expiry, most faded — least visible pop)
    local bestIdx = 1
    local bestT   = -1.0
    for i = 1, POOL_SIZE do
        local t = pool[i].elapsed / pool[i].lifetime
        if t > bestT then
            bestT   = t
            bestIdx = i
        end
    end
    HideSlot(pool[bestIdx])
    return bestIdx
end

-- ─────────────────────────────────────────────────────────────────────────────
--  START
-- ─────────────────────────────────────────────────────────────────────────────
function Start()
    -- Collect pre-built pool children.
    -- GetChildrenList returns all direct children of this entity (feedback cnv).
    local children = GetChildrenList(EntityID)

    for i = 1, POOL_SIZE do
        local ent = children[i]   -- 1-indexed; sol2 wraps std::vector with 1-based indexing
        if not ent then
            Log("[FeedbackBehaviour] Warning: fewer pre-built pool children than POOL_SIZE ("
                .. tostring(POOL_SIZE) .. "). Got " .. tostring(#children))
            -- Pad remaining slots with invalid sentinel
            pool[i] = { entity = -1, alive = false, elapsed = 0, lifetime = LIFETIME,
                        basePixX = 0, basePixY = 0, jitterPixX = 0,
                        baseFontSize = FONT_SIZE_NORMAL, fall = false }
        else
            pool[i] = {
                entity       = ent,
                alive        = false,
                elapsed      = 0.0,
                lifetime     = LIFETIME,
                basePixX     = 0.0,
                basePixY     = 0.0,
                jitterPixX   = 0.0,
                baseFontSize = FONT_SIZE_NORMAL,
                fall         = false,
            }
            -- Initialise text component defaults
            local txt = GetTextFrom(ent)
            if txt then
                txt.text    = ""
                txt.visible = false
                txt.color.a = 0.0
            end
            SetActiveEntity(ent, false)
        end
    end

    -- Register the Lua-side spawn bridge so C++ SpawnFeedback() can route here.
    -- C++ RegisterFeedbackAPI should call: (*sharedLua)["_FeedbackSpawn"](worldX, worldY, value, typeStr)
    _G["_FeedbackSpawn"] = function(pixX, pixY, value, typeStr)
        Spawn(pixX, pixY, value, typeStr or "normal")
    end

    Log("[FeedbackBehaviour] Start() — pool ready, " .. tostring(#children) .. " children found.")
end

-- ─────────────────────────────────────────────────────────────────────────────
--  SPAWN  (called by _FeedbackSpawn bridge; pixX/pixY are already screen-pixel
--          coords — C++ RegisterFeedbackAPI must convert worldX/worldY →
--          screen pixels via pGraphics->WorldToScreen() before calling in)
-- ─────────────────────────────────────────────────────────────────────────────
function Spawn(pixX, pixY, value, typeStr)
    -- Resolve type string → config
    local displayText = value or ""
    local color       = COLOR.Normal
    local fontSize    = FONT_SIZE_NORMAL
    local fall        = false

    local t = string.lower(typeStr or "normal")

    if t == "affinity" then
        displayText = displayText .. "!"
        color       = COLOR.Affinity
        fontSize    = FONT_SIZE_AFFINITY

    elseif t == "crit" or t == "critical" then
        displayText = displayText .. "!!"
        color       = COLOR.Critical
        fontSize    = FONT_SIZE_CRIT

    elseif t == "heal" then
        color    = COLOR.Heal
        fontSize = FONT_SIZE_NORMAL

    elseif t == "playerhit" or t == "player" then
        color    = COLOR.PlayerHit
        fontSize = FONT_SIZE_NORMAL

    elseif t == "manaspend" or t == "mana" then
        color    = COLOR.ManaSpend
        fontSize = FONT_SIZE_MANA
        fall     = true

    elseif t == "managain" then
        color    = COLOR.ManaGain
        fontSize = FONT_SIZE_MANA

    elseif t == "warn" or t == "warning" then
        color    = COLOR.Warning
        fontSize = FONT_SIZE_WARNING
        fall     = true
    end

    -- Jitter
    local jitter = (NextJitter() * 2.0 - 1.0) * SPREAD_RADIUS

    local idx  = AcquireSlot()
    local slot = pool[idx]

    -- Guard: invalid entity (pool was built with fewer children than POOL_SIZE)
    if slot.entity == -1 then
        LogWarning("[FeedbackBehaviour] Spawn skipped — slot " .. idx .. " has no entity.")
        return
    end

    slot.alive        = true
    slot.elapsed      = 0.0
    slot.lifetime     = LIFETIME
    slot.basePixX     = pixX
    slot.basePixY     = pixY
    slot.jitterPixX   = jitter
    slot.baseFontSize = fontSize
    slot.fall         = fall

    -- Write initial component state
    local txt = GetTextFrom(slot.entity)
    if txt then
        txt.text      = displayText
        txt.fontSize  = fontSize * PUNCH_PEAK
        txt.color.r   = color.r
        txt.color.g   = color.g
        txt.color.b   = color.b
        txt.color.a   = 1.0
        txt.visible   = true
    end

    local rt = GetRectTransformFrom(slot.entity)
    if rt then
        rt.anchoredPosition.x = pixX + jitter
        rt.anchoredPosition.y = pixY
        rt.sizeDelta.x        = fontSize * PUNCH_PEAK * 5.0
        rt.sizeDelta.y        = fontSize * PUNCH_PEAK * 2.0
        rt.isDirty            = true
    end

    SetActiveEntity(slot.entity, true)
end

-- ─────────────────────────────────────────────────────────────────────────────
--  UPDATE
-- ─────────────────────────────────────────────────────────────────────────────
function Update(dt)
    for i = 1, POOL_SIZE do
        local slot = pool[i]
        if not slot.alive then goto continue end
        if slot.entity == -1 then goto continue end

        slot.elapsed = slot.elapsed + dt
        local frac = slot.elapsed / slot.lifetime   -- 0 → 1

        -- Expire
        if frac >= 1.0 then
            HideSlot(slot)
            goto continue
        end

        -- Vertical drift (pixels)
        local vertOffset
        if slot.fall then
            vertOffset = -(FALL_SPEED * slot.elapsed)
        else
            vertOffset =  (RISE_SPEED * slot.elapsed)
        end

        -- Fade-out
        local alpha = 1.0
        if frac > FADE_START then
            local fadeSpan = 1.0 - FADE_START
            alpha = 1.0 - ((frac - FADE_START) / fadeSpan)
            if alpha < 0.0 then alpha = 0.0 end
        end

        -- Scale punch (ease-out quad)
        local scaleMul = 1.0
        if slot.elapsed < PUNCH_DURATION then
            local punchT = slot.elapsed / PUNCH_DURATION
            local eased  = 1.0 - (1.0 - punchT) * (1.0 - punchT)
            scaleMul = 1.0 + (PUNCH_PEAK - 1.0) * eased
        end

        -- Write RectTransform
        local rt = GetRectTransformFrom(slot.entity)
        if rt then
            rt.anchoredPosition.x = slot.basePixX + slot.jitterPixX
            rt.anchoredPosition.y = slot.basePixY + vertOffset
            rt.sizeDelta.x        = slot.baseFontSize * scaleMul * 5.0
            rt.sizeDelta.y        = slot.baseFontSize * scaleMul * 2.0
            rt.isDirty            = true
        end

        -- Write Text
        local txt = GetTextFrom(slot.entity)
        if txt then
            txt.color.a  = alpha
            txt.fontSize = slot.baseFontSize * scaleMul
        end

        ::continue::
    end
end