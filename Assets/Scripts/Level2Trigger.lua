local children
local collider
local missionTextComponent
local missionTextComponent2
local triggerCount
local blockerCollider = {}
local TotalEnemyCount

ExposedVars = {
    missionText = "",
    moveToNext = "",

    Room1EnemyCount = 0,
    Room2EnemyCount = 0,
    Room3EnemyCount = 0,
}

function Start()
    if HasCollider() then
        collider = GetCollider()
    end

    triggerCount = 0

    children = GetChildrenList(EntityID)
    -- stage blockers ref
    blockerCollider[1] = children[6]
    blockerCollider[2] = children[7]
    blockerCollider[3] = children[8]

    -- mission text refs
    missionTextComponent = GetTextFrom(children[3])
    missionTextComponent2 = GetTextFrom(children[4])

    TotalEnemyCount = CountEntitiesWithComponent("Enemy")
end

function OnTriggerEnter()
    -- disable collider so cannot retrigger the same mission
    -- change text for 1st mission
    if triggerCount == 0 then
        RoomTriggerInit()
        SetMissionText(missionText)

    -- change text for 2nd mission
    elseif triggerCount == 1 then
        RoomTriggerInit()
        SetMissionText(missionText)

    -- change text for 3rd mission
    elseif triggerCount == 2 then
        RoomTriggerInit()
        SetMissionText(missionText)

    -- change text for 4th mission
    elseif triggerCount == 3 then
        RoomTriggerInit()
        SetMissionText("touch the kappa statue")
    end
end

function Update(dt)
    local CurrEnemyCount = CountEntitiesWithComponent("Enemy")
    local KilledEnemies = TotalEnemyCount - CurrEnemyCount

    if triggerCount == 1 then
        if KilledEnemies >= math.floor(Room1EnemyCount) then
            SetMissionText(moveToNext)
            SetActiveEntity(blockerCollider[1], false)
        end
    end
    -- Room 2
    if triggerCount == 2 then
        if KilledEnemies >= (math.floor(Room1EnemyCount) + math.floor(Room2EnemyCount)) then
            SetMissionText(moveToNext)
            SetActiveEntity(blockerCollider[2], false)
        end
    end
    -- Room 3
    if triggerCount == 3 then
        if KilledEnemies >= (math.floor(Room1EnemyCount) + math.floor(Room2EnemyCount) + math.floor(Room3EnemyCount)) then
            SetMissionText(moveToNext)
            SetActiveEntity(blockerCollider[3], false)
        end
    end
end

function RoomTriggerInit()
    collider.shapes[triggerCount+1].isActive = false
    triggerCount = triggerCount + 1
end

function SetMissionText(str)
    missionTextComponent.text = str
    missionTextComponent2.text = str
end