ExposedVars = {
    StunnedDuration = 10.0
}

local StunCD = 0.0

--takes in entity id from C++ to use in case needed
function state_enter(entity)
    Log("stunned state entered")
    StunCD = StunnedDuration
end

--takes in entity id from C++ to use in case needed
--uses dt for time updates etc
function state_update(entity, dt)
    StunCD = StunnedDuration - dt;
    if StunCD < 0 then
        --set to idle for now so if attack conditions are met in it will transition to attack
        ChangeState(entity, "WaterDemonIdle") 
    end
end

--takes in entity id from C++ to use in case needed
function state_exit(entity)
    Log("stunned state exited")
end