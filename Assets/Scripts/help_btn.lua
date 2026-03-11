-- function OnPointerEnter()
--     local child = GetChildren(EntityID, 0)
--     local text = GetTextFrom(child)
--     text.color.r = 0.953
--     text.color.g = 0.859
--     text.color.b = 0.757
--     text.color.a = 1.0
-- end

-- function OnPointerExit()
--     local child = GetChildren(EntityID, 0)
--     local text = GetTextFrom(child)
--     text.color.r = 0.792
--     text.color.g = 0.651
--     text.color.b = 0.584
--     text.color.a = 1.0
-- end

function OnPointerClick()
    local canvas = GetParent(EntityID)
    local mainBg = GetChildren(canvas, 0)
    local playBtn = GetChildren(canvas, 1)
    local helpBtn = GetChildren(canvas, 2)
    local credBtn = GetChildren(canvas, 3)
    local quitBtn = GetChildren(canvas, 4)
    local helpMnl = GetChildren(canvas, 5)
    SetActiveEntity(mainBg, false)
    SetActiveEntity(playBtn, false)
    SetActiveEntity(helpBtn, false)
    SetActiveEntity(credBtn, false)
    SetActiveEntity(quitBtn, false)
    SetActiveEntity(helpMnl, true)
end