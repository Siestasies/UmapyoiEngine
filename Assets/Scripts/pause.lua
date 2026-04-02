MenuNav = dofile("Assets/Scripts/menu_nav.lua")
local nav

local isOptionTurnedOn
local optionScrn

local controllerInputIndicator
local isControllerInputIndicatorActive

function Start()

    local children = GetChildrenList(EntityID)
    -- children[1..4] are your pause buttons in order
    -- adjust indices to match which buttons you want navigable
    nav = MenuNav.new(
        { children[2], children[3], children[4] },
        { backButton = children[2], wrapAround = true }
    )

    nav:setActive(false)  -- start inactive until paused

    isOptionTurnedOn = false;

    local gm_id = GetParent(EntityID)
    optionScrn = GetChildren(gm_id, 4)

    controllerInputIndicator = children[5]
    isControllerInputIndicatorActive = false

end

function Update(dt)
    
    isOptionTurnedOn = GetActiveEntity(optionScrn)

    if GetCurrentInputMethod() == 1 and not isControllerInputIndicatorActive then
        SetActiveEntity(controllerInputIndicator, true) 
        isControllerInputIndicatorActive = true
    elseif GetCurrentInputMethod() == 0 and isControllerInputIndicatorActive then
        SetActiveEntity(controllerInputIndicator, false) 
        isControllerInputIndicatorActive = false
    end

    if isOptionTurnedOn and nav:getActive() == true then 
        nav:setActive(false)
    elseif not isOptionTurnedOn and nav:getActive() == false then
        Focused(2)
    end

    nav:update(dt)

end

function OnEnable()
    Focused(1)
    Log("pause enabled")
end

function OnDisable()
    nav:setActive(false)
    Log("pause disabled")
end

function Focused(index)
    nav:setActive(true)
    nav:setFocused(index)
end