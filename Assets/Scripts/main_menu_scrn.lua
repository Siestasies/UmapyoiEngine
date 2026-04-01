MenuNav = dofile("Assets/Scripts/menu_nav.lua")
local nav

local isHelpTurnedOn
local helpScrn

local isCfmTurnedOn
local cfmScrn

function Start()

    local children = GetChildrenList(EntityID)
    -- children[1..4] are your pause buttons in order
    -- adjust indices to match which buttons you want navigable
    nav = MenuNav.new(
        { children[2], children[3], children[4], children[5]},
        { backButton = children[5], wrapAround = false }
    )

    nav:setActive(false)  -- start inactive until paused

    isHelpTurnedOn = false;

    helpScrn = children[6]

    isCfmTurnedOn = false;

    cfmScrn = children[7]

end

function Update(dt)
    
    isHelpTurnedOn = GetActiveEntity(helpScrn)
    isCfmTurnedOn = GetActiveEntity(cfmScrn)


    if (isHelpTurnedOn or isCfmTurnedOn) and nav:getActive() == true then 
        nav:setActive(false)
    elseif not isHelpTurnedOn and nav:getActive() == false then
        Focused(3)
    elseif not isCfmTurnedOn and nav:getActive() == false then
        Focused(4)
    end

    nav:update(dt)

end

function OnEnable()
    Focused(1)
end

function OnDisable()
    nav:setActive(false)
end

function Focused(index)
    nav:setActive(true)
    nav:setFocused(index)
end