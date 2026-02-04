bool = true

function OnClick()

    PlaySound("btn_clicked", 1.0, 0)
    SetActiveEntity(EntityID, false)


    
    
    
   
    
    

    
    
end

function Update(dt)
    local parent = GetParent(EntityID)
    local grandparent = GetParent(parent)
     
    local children = GetChildrenList(EntityID)
    local parentChildren = GetChildrenList(parent)
    local grandChildren = GetChildrenList(grandparent)
    
    local paused = IsGamePause()
    if paused == true then 
        SetActiveEntity(EntityID, false)
    end

    if bool ==  true then
        if #children > 0 then
            local child = children[1]
            SetActiveEntity(child, true)
        end

        if #children >= 2 then
            local child = children[2]
            SetActiveEntity(child, true)
        end

        if #children >= 3 then
            local child = children[3]
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

        if #children >= 3 then
            local child = children[3]
            SetActiveEntity(child, false)
        end
        
    end

    if bool == true then
        if #parentChildren > 0 then
            local child = parentChildren[1]
            SetActiveEntity(child, false)
        end

        if #parentChildren >= 2 then
            local child = parentChildren[2]
            SetActiveEntity(child, false)
        end

        if #parentChildren >= 3 then
            local child = parentChildren[3]
            SetActiveEntity(child, true)
        end
        
    else
        if #parentChildren > 0 then
            local child = parentChildren[1]
            SetActiveEntity(child, false)
        end

        if #parentChildren >= 2 then
            local child = parentChildren[2]
            SetActiveEntity(child, false)
        end

        if #parentChildren >= 3 then
            local child = parentChildren[3]
            SetActiveEntity(child, true)
        end
        
    end
    if bool == true then
        if #grandChildren > 0 then
            local child = grandChildren[1]
            SetActiveEntity(child, false)
        end

        if #grandChildren >= 2 then
            local child = grandChildren[2]
            SetActiveEntity(child, false)
        end

        if #grandChildren >= 3 then
            local child = grandChildren[3]
            SetActiveEntity(child, false)
        end
        
        
    else
        if #grandparent > 0 then
            local child = grandparent[1]
            SetActiveEntity(child, false)
        end

        if #grandparent >= 2 then
            local child = grandparent[2]
            SetActiveEntity(child, false)
        end

        if #grandparent >= 3 then
            local child = grandparent[3]
            SetActiveEntity(child, true)
        end
        
        
    end
    



end

