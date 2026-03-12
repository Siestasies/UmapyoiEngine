local total
local children
local lastThreshold = -1

function Start()
    children = GetChildrenList(EntityID)
    total = #children
end

function Update()
    if total == 0 then return end

    local animator = GetAnimator()
    children = GetChildrenList(EntityID)
    local current = #children

    local percent = (current / total) * 100  -- gives 0 to 100
    local threshold = math.floor(percent / 25)  -- gives 0, 1, 2, 3, or 4

    if threshold ~= lastThreshold then
        lastThreshold = threshold

        if threshold == 3 then
            animator.animator:Play("eye1", false)  -- 75% enemies remain
        elseif threshold == 2 then
            animator.animator:Play("eye2", false)  -- 50% remain
        elseif threshold == 1 then
            animator.animator:Play("eye3", false)  -- 25% remain
        elseif threshold == 0 then
            animator.animator:Play("eye4", false)  -- all dead
        end
    end
end