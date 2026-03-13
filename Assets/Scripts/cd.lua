local playerEntity = -1

function Start()
    -- Cross-entity access
    playerEntity = FindEntityWithComponent("Player")
end

function Update(dt)
    for i = 0, 2 do
        if HasChildren(EntityID, i) then
            local skill = GetChildren(EntityID, i)
            if HasChildren(skill, 0) then
                local fill = GetChildren(skill, 0)

                GetImageFrom(fill).fillAmount = 0
            end
        end
    end
end