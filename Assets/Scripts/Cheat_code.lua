function Start()
    
end

function Update(dt)
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


end