MenuNav = dofile("Assets/Scripts/menu_nav.lua")
local nav

function Start()

    local children = GetChildrenList(EntityID)
    nav = MenuNav.new(
        { children[1] },
        { backButton = children[1], wrapAround = true }
    )

    
    nav:setActive(false)  -- start inactive until paused

end

function Update(dt)
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