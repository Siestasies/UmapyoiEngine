local effects
local children

function Start()
    effects = GetEffectsFrom(EntityID)
    children = GetChildrenList(EntityID)
end

function Update(dt)
    if effects and (effects:IsClipComplete(0) and effects:IsClipComplete(1)) then
        SetActiveEntity(children[1], true)
        SetActiveEntity(children[2], true)
    end
end