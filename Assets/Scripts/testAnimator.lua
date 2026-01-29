local animator;

function Start()

    if HasAnimator() then 
    
        animator = GetAnimator()
    
    end

end

function Update(dt)
    if KeyPressed(KEY_E) then
        Log("curr animation : " .. animator.animator:GetCurrentClip())
    end
end