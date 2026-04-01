MenuNav = dofile("Assets/Scripts/menu_nav.lua")
local nav

local isOptionTurnedOn
local optionScrn

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

end

function Update(dt)
    
    isOptionTurnedOn = GetActiveEntity(optionScrn)

    if isOptionTurnedOn and nav:getActive() == true then 
        nav:setActive(false)
    elseif not isOptionTurnedOn and nav:getActive() == false then
        Focused()
    end

    nav:update(dt)

end

function OnEnable()
    Focused()
    Log("pause enabled")
end

function OnDisable()
    nav:setActive(false)
    Log("pause disabled")
end

function Focused()
    nav:setActive(true)
    nav:setFocused(1)
end