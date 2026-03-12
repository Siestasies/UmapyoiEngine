function Start()

    StartPlayTime(true)

end

function Update(dt)

    local playTime = GetPlayTime()
    Log("Curr play time : " .. playTime)

end