--this master script should purely handle entity wide behavior like collision logic
--or health tracking stuff and force transition only if it is required to
--state specific stuff should stay with states
--examples like taking damage/collision with other entities should be in master script
--walking, attack, running should be its own script
ExposedVars = {

}

--default functions
function Start()
    
end

function Update(dt)
    
end

function OnDestroy()
    
end

--optional use as needed
function OnCollisionEnter(other)
    
end

function OnCollisionExit(other)

end

function OnTriggerEnter(other)
    
end

function OnTriggerExit(other)
    
end
