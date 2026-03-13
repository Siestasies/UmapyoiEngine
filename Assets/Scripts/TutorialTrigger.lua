local children
local collider
local missionTextComponent
local missionTextComponent2
local PlayerStatTrackState = require("PlayerStatTrackState")
local triggerCount
local blockerCollider = {}
local tutorialEnemies = {}

ExposedVars = {
    missionText = "",
    missionText2 = "",
    missionText3 = "",
    missionText4 = "",
    missionText5 = "",
    missionText6 = "",
    missionText7 = "",
    moveToNext = "",
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
    blockerCollider[4] = children[9]

    -- static enemy refs
    tutorialEnemies[1] = children[10]
    tutorialEnemies[2] = children[11]

    -- mission text refs
    missionTextComponent = GetTextFrom(children[3])
    missionTextComponent2 = GetTextFrom(children[4])

    PlayerStatTrackState.SetFireAttackCount(0)
    PlayerStatTrackState.SetWaterAttackCount(0)
    PlayerStatTrackState.SetWindAttackCount(0)
    PlayerStatTrackState.SetPyronadoAttackCount(0)
    PlayerStatTrackState.SetWhirlpoolAttackCount(0)
    PlayerStatTrackState.SetSteamburstAttackCount(0)
    PlayerStatTrackState.SetPassedTrigger(0)
    
    --SpawnPrefab("Tutorial Popup1.prefab", Vec2(0,0))
end

function OnTriggerEnter()
    -- disable collider so cannot retrigger the same mission
    -- change text for 1st mission
    if triggerCount == 0 then
        RoomTriggerInit()
        SetMissionText(missionText)
        SpawnPrefab("Tutorial Popup2.prefab", Vec2(0,0))
    -- change text for 2nd mission
    elseif triggerCount == 1 then
        RoomTriggerInit()
        SetMissionText(missionText4)
        SpawnPrefab("Tutorial Popup3.prefab", Vec2(0,0))
    -- change text for 3rd mission
    elseif triggerCount == 2 then
        RoomTriggerInit()
        SetMissionText(missionText7)

    -- change text for 4th mission
    elseif triggerCount == 3 then
        RoomTriggerInit()
        SetMissionText(missionText7)
    end
end

function Update(dt)
    -- update missions to cast each skill once, sequentially for trigger 1,2
    -- eg, cast fire, cast water, cast wind, next level
    -- trigger 3 all enemy die
    -- trigger 4 all enemy die

    -- if player use each elemental skills at least 3x
    if PlayerStatTrackState.GetPassedTrigger() == 1 then
        local fire = PlayerStatTrackState.GetFireAttackCount()
        local water = PlayerStatTrackState.GetWaterAttackCount()
        local wind = PlayerStatTrackState.GetWindAttackCount()

        if wind >= 3 and water >= 3 and fire >= 3 then
            SetMissionText(moveToNext)
            SetActiveEntity(blockerCollider[1], false)
            DestroyWithChildren(tutorialEnemies[1])
        elseif water >= 3 and fire >= 3 then
            SetMissionText(missionText3 .. (3-wind) .. " times")  -- wind
        elseif fire >= 3 then
            SetMissionText(missionText2 .. (3-water) .. " times")  -- water
        else 
            SetMissionText(missionText .. (3-fire) .. " times")  -- fire
        end
        -- missionText use fire set in OnTriggerEnter

    elseif PlayerStatTrackState.GetPassedTrigger() == 2 then
        local pyro = PlayerStatTrackState.GetPyronadoAttackCount()
        local whirl = PlayerStatTrackState.GetWhirlpoolAttackCount()
        local steam = PlayerStatTrackState.GetSteamburstAttackCount()

        if steam >= 3 and whirl >= 3 and pyro >= 3 then
            SetMissionText(moveToNext)
            SetActiveEntity(blockerCollider[2], false)
            DestroyWithChildren(tutorialEnemies[2], false)
        elseif whirl >= 3 and pyro >= 3 then
            SetMissionText(missionText6 .. (3-steam) .. " times")  -- steamburst
        elseif pyro >= 3 then
            SetMissionText(missionText5 .. (3-whirl) .. " times")  -- whirlpool
        else 
            SetMissionText(missionText4 .. (3-pyro) .. " times")  -- pyronado
        end

    -- when only specifically <<2>> enemies left
    -- assuming player killed 1 at the third tutorial
    elseif PlayerStatTrackState.GetPassedTrigger() == 3 then
        local enemyCount = CountEntitiesWithComponent("Enemy")
        if enemyCount == 2 then
            SetActiveEntity(blockerCollider[3], false)
            SetMissionText(moveToNext)
        end

    -- when only specifically <<0>> enemies left
    -- assuming player killed last 2 at the fourth tutorial
    elseif PlayerStatTrackState.GetPassedTrigger() == 4 then
        local enemyCount = CountEntitiesWithComponent("Enemy")
        if enemyCount == 0 then
            SetActiveEntity(blockerCollider[4], false)
            SetMissionText("Touch the Kappa Shrine")
        end
    end

end

function RoomTriggerInit()
    collider.shapes[triggerCount+1].isActive = false
    PlayerStatTrackState.incrPassedTrigger()
    triggerCount = triggerCount + 1
end

function SetMissionText(str)
    missionTextComponent.text = str
    missionTextComponent2.text = str
end