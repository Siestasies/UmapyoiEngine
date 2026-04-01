MenuNav = dofile("Assets/Scripts/menu_nav.lua")
local nav

function Start()

    local children = GetChildrenList(EntityID)
    -- children[1..4] are your pause buttons in order
    -- adjust indices to match which buttons you want navigable
    nav = MenuNav.new(
        { children[2], children[3], children[4] },
        { wrapAround = true }
    )

    Log("child : " .. children[2])
    Log("child : " .. children[3])
    Log("child : " .. children[4])


    nav:setActive(false)  -- start inactive until paused

end

function Update(dt)
    --Log("pause is updating")

    nav:setActive(true)
    nav:update(dt)

end