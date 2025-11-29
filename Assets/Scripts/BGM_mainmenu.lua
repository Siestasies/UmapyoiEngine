local start
function Start()
    start = false
end

function Update(dt)
    if start == false then
        PlaySound("mainmenu_bgm", 0.2, -1); 
        start = true
    end
end