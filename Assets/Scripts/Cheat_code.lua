local PlayerRef = nil
local GodMode = false
local SecretToggle = false

function Start()
    PlayerRef = GetPlayerFrom(FindEntityWithComponent("Player"))
    GodMode = false
end

function Update(dt)
    if KeyDown(KEY_LALT) and KeyDown(KEY_RALT) and not bothAltsActive then
        SecretToggle = not SecretToggle
        bothAltsActive = true
    elseif not (KeyDown(KEY_LALT) and KeyDown(KEY_RALT)) then
        bothAltsActive = false
    end

    if not SecretToggle then return end

    --transition the levels
    if KeyReleased(KEY_1) then
        LoadScene("tutorial_v3.scn")
    elseif KeyReleased(KEY_2) then
        LoadScene("level_1_v3.scn")
    elseif KeyReleased(KEY_3) then
        LoadScene("level_2_v3.scn")
    elseif KeyReleased(KEY_4) then
        LoadScene("level_3_v3.scn")
    elseif KeyReleased(KEY_5) then
        LoadScene("boss_map.scn")
    end

    --player cheats
    --god mode
    if KeyReleased(KEY_0) then
        GodMode = not GodMode
        if GodMode == true then
            PlayerRef.mHealth = 100000
            PlayerRef.mMana = 100000
            PlayerRef.mAttackDamage = 100000
            PlayerRef.mMana = 1000000000000000000000000
        else
            PlayerRef.mHealth = PlayerRef.mMaxHealth
            PlayerRef.mMana = PlayerRef.mMaxMana
            PlayerRef.mAttackDamage = 40
        end
    end

    --clear all enemies
    if KeyReleased(KEY_9) then
        local enemies = FindEntitiesWithComponent("Enemy")
        for i = 1, #enemies do
            local enemy = GetEntity(enemies[i])
            if not enemy.isValid then return end
            enemy:GetEnemy().mHealth = 0
        end
    end

end