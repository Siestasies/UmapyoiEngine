local MenuNav = {}
MenuNav.__index = MenuNav

function MenuNav.new(buttons, opts) 

    -- buttons: ordered list of button entity IDs
    -- opts: { backButton = entity, wrapAround = true }
    local self = setmetatable({}, MenuNav)
    self.buttons = buttons
    self.focusIndex = 1
    self.backButton = opts and opts.backButton
    self.wrapAround = opts == nil or opts.wrapAround ~= false
    self.isHorizontal = opts.isHorizontal ~= nil and opts.isHorizontal ~= false
    self.active = true
    -- cooldown to prevent rapid scrolling
    self.inputCooldown = 0
    self.cooldownTime = 0.2

    return self 

end

function MenuNav:update(dt)
    if not self.active then return end

    if self.skipFrame then
        self.skipFrame = false
        return
    end

    --self.inputCooldown = self.inputCooldown - dt
    --if self.inputCooldown > 0 then return end

    -- Navigate down
    if (not self.isHorizontal and GetControllerButtonInput(BTN_DOWN, BTN_PRESS, 0) or
    self.isHorizontal and GetControllerButtonInput(BTN_RIGHT, BTN_PRESS, 0))
    and self.focusIndex ~= -1 then
        local next = self.focusIndex + 1

        if next > #self.buttons then
            next = self.wrapAround and 1 or #self.buttons
        end

        local ok, err = pcall(function() self:setFocused(next) end)
        if not ok then Log("setFocused error: " .. tostring(err)) end
        
        self.inputCooldown = self.cooldownTime

    -- Navigate up
    elseif (not self.isHorizontal and GetControllerButtonInput(BTN_UP, BTN_PRESS, 0) or
    self.isHorizontal and GetControllerButtonInput(BTN_LEFT, BTN_PRESS, 0))
    and self.focusIndex ~= -1 then
        local next = self.focusIndex - 1
        if next < 1 then
            next = self.wrapAround and #self.buttons or 1
        end

        local ok, err = pcall(function() self:setFocused(next) end)
        if not ok then Log("setFocused error: " .. tostring(err)) end

        self.inputCooldown = self.cooldownTime
    end

    -- Confirm
    if GetControllerButtonInput(BTN_A, BTN_PRESS, 0) and self.focusIndex ~= -1 then
        SimulateButtonAction(self.buttons[self.focusIndex], ButtonState.Pressed)
        self.active = false
        return
    end

    -- Back
    if GetControllerButtonInput(BTN_B, BTN_PRESS, 0) and self.focusIndex ~= -1 then
       SimulateButtonAction(self.backButton, ButtonState.Pressed)
        self.active = false
        return
    end
end

function MenuNav:setFocused(index)

    if index == -1 then return end
    for i, id in ipairs(self.buttons) do
        SimulateButtonAction(id, ButtonState.Normal)
    end

    local currBtn = GetButtonFrom(self.buttons[index])
    if currBtn ~= nil then
        SimulateButtonAction(self.buttons[index], ButtonState.Hovered)
        self.focusIndex = index
    end

    --Log("curr target " .. self.focusIndex)
end

function MenuNav:setActive(active)
    self.active = active
    if active then
        self.skipFrame = true  -- skip one frame of input
    end
end

function MenuNav:getActive()
    return self.active
end

return MenuNav