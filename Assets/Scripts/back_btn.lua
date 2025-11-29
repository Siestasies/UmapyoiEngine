local time = 0
local clicked = false

function OnClicked()
    PlaySound("btn_clicked", 0.8, 0)
    clicked = true
end

function Update(dt)
    if clicked then
        time = time + 0.1
        if time > 1 then
            time = 0
            clicked = false
            PauseGame(false)
            LoadScene("main_menu.scn")
        end
    end
end