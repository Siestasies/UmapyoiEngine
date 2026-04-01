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
    self.active = true
    -- cooldown to prevent rapid scrolling
    self.inputCooldown = 0
    self.cooldownTime = 0.2

    return self 

end

function MenuNav:update(dt)
    if not self.active then return end

    Log("curr update target " .. self.focusIndex)

    --self.inputCooldown = self.inputCooldown - dt
    --if self.inputCooldown > 0 then return end

    -- Navigate down
    if GetControllerButtonInput(BTN_DOWN, BTN_PRESS, 0) or
       GetControllerAxesInput(AXIS_LEFT_Y, 0) > 0.5 then
        local next = self.focusIndex + 1

        Log("curr target " .. next .. " and num button " .. #self.buttons)

        if next > #self.buttons then
            next = self.wrapAround and 1 or #self.buttons
        end

        local ok, err = pcall(function() self:setFocused(next) end)
        if not ok then Log("setFocused error: " .. tostring(err)) end
        
        self.inputCooldown = self.cooldownTime

    -- Navigate up
    elseif GetControllerButtonInput(BTN_UP, BTN_PRESS, 0) or
           GetControllerAxesInput(AXIS_LEFT_Y, 0) < -0.5 then
        local next = self.focusIndex - 1
        if next < 1 then
            next = self.wrapAround and #self.buttons or 1
        end

        local ok, err = pcall(function() self:setFocused(next) end)
        if not ok then Log("setFocused error: " .. tostring(err)) end

        self.inputCooldown = self.cooldownTime
    end

    -- Confirm
    if GetControllerButtonInput(BTN_A, BTN_PRESS, 0) then
         Log("curr target " .. self.focusIndex)
        SimulateButtonAction(self.buttons[self.focusIndex], ButtonState.Pressed)
    end

    -- Back
    if GetControllerButtonInput(BTN_B, BTN_PRESS, 0) and self.backButton then
         Log("curr target " .. self.focusIndex)
       SimulateButtonAction(self.backButton, ButtonState.Pressed)
    end
end

function MenuNav:setFocused(index)

    for i, id in ipairs(self.buttons) do
        SimulateButtonAction(id, ButtonState.Normal)
    end

    local currBtn = GetButtonFrom(self.buttons[index])
    if currBtn ~= nil then
        SimulateButtonAction(self.buttons[index], ButtonState.Hovered)
        self.focusIndex = index
    end

    Log("curr target " .. self.focusIndex)
end

function MenuNav:setActive(active)
    self.active = active
end

return MenuNav