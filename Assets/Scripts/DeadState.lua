ExposedVars = {

}

function state_enter(entity)
    Log("dead state entered")
end

function state_update(entity, dt)
    --play animation 
    --log score/do drops 
    DestroyWithChildren(entity)
end

function state_exit(entity)
    Log("dead state exited")
end