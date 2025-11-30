local bool = true


function OnClicked()
    local parent = GetParent(EntityID)
    local children = GetChildren(EntityID)
    
    local parentChildren = GetChildren(parent)
    

    
        

        
            
    
        
        
    
        if #children >= 3 then
            local child = children[3]
            SetActiveEntity(child, true)
        end
        
    

    
    
    PlaySound("btn_clicked", 1.0, 0)
   
    
    

    
    
end


function Update(dt)


    local children = GetChildren(EntityID)

    local paused = IsGamePause()

    if paused == true then
        if #children > 0 then
            local child = children[1]
            SetActiveEntity(child, true)
        end

        if #children >= 2 then
            local child = children[2]
            SetActiveEntity(child, true)
        end

        
        
    else
        if #children > 0 then
            local child = children[1]
            SetActiveEntity(child, false)
        end

        if #children >= 2 then
            local child = children[2]
            SetActiveEntity(child, false)
        end

        
        
    end

    
end

