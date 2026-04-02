MenuNav = dofile("Assets/Scripts/menu_nav.lua")
local nav

local controllerInputIndicator
local isControllerInputIndicatorActive

function Start()

    local children = GetChildrenList(EntityID)
    nav = MenuNav.new(
        { children[3] },
        { backButton = children[3], wrapAround = true }
    )

    
    nav:setActive(false)  -- start inactive until paused

    controllerInputIndicator = children[4]
    isControllerInputIndicatorActive = false

end

function Update(dt)

    if GetCurrentInputMethod() == 1 and not isControllerInputIndicatorActive then
        SetActiveEntity(controllerInputIndicator, true) 
        isControllerInputIndicatorActive = true
    elseif GetCurrentInputMethod() == 0 and isControllerInputIndicatorActive then
        SetActiveEntity(controllerInputIndicator, false) 
        isControllerInputIndicatorActive = false
    end

    nav:update(dt)

end

function OnEnable()
    Focused()
end

function OnDisable()
    nav:setActive(false)
end

function Focused()
    nav:setActive(true)
    nav:setFocused(1)
end