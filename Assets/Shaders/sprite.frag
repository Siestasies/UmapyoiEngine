#version 450 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D image;
uniform vec3 tintColor;
uniform float alpha;

// UV clipping uniforms
uniform int fillDirection;  // 0=None, 1=LeftToRight, 2=RightToLeft, 3=TopToBottom, 4=BottomToTop
uniform float fillAmount;   // 0.0 to 1.0

void main()
{
    // Discard fragments based on fill direction
    if (fillDirection == 1) // LeftToRight
    {
        if (TexCoords.x > fillAmount) discard;
    }
    else if (fillDirection == 2) // RightToLeft
    {
        if (TexCoords.x < (1.0 - fillAmount)) discard;
    }
    else if (fillDirection == 3) // TopToBottom
    {
        if (TexCoords.y > fillAmount) discard;
    }
    else if (fillDirection == 4) // BottomToTop
    {
        if (TexCoords.y < (1.0 - fillAmount)) discard;
    }
    vec4 texColor = texture(image, TexCoords);
    color = vec4(texColor.rgb * tintColor, texColor.a * alpha);
}
