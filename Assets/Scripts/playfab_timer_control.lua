ExposedVars =
{
    timer_mode = 1,
    debug = 0
}


function Start()

    StartPlayTime(timer_mode == 1)

end

function Update(dt)

    if debug == 1 then 
        local playTime = GetPlayTime()
        Log("Curr play time : " .. playTime)
    end
end