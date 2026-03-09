local animator

function Start()
    animator = GetAnimator()
end

function Update()
    if KeyDown(KEY_I) then
        animator.animator:Play( "eye1", false)
    elseif KeyDown(KEY_O) then
        animator.animator:Play( "eye2", false)
    elseif KeyDown(KEY_P) then
        animator.animator:Play( "eye3", false)
    end
end