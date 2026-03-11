#version 450 core
in vec2 TexCoords;
in vec2 RawTexCoords;
in vec4 Tint;
out vec4 color;

uniform sampler2D image;

// Fill effect - clips against raw [0,1] quad UVs, not the spritesheet-transformed ones
uniform int   fillDirection; // 0=None, 1=LeftToRight, 2=RightToLeft, 3=TopToBottom, 4=BottomToTop
uniform float fillAmount;    // 0.0 to 1.0

void main()
{
    // Fill clipping uses RawTexCoords so it always operates over the full [0,1]
    // range of the quad regardless of which spritesheet cell is selected
    if (fillDirection == 1 && RawTexCoords.x > fillAmount) discard;          // Left to Right
    else if (fillDirection == 2 && RawTexCoords.x < 1.0 - fillAmount) discard; // Right to Left
    else if (fillDirection == 3 && RawTexCoords.y > fillAmount) discard;      // Top to Bottom
    else if (fillDirection == 4 && RawTexCoords.y < 1.0 - fillAmount) discard; // Bottom to Top

    vec4 texColor = texture(image, TexCoords);
    color = vec4(texColor.rgb * Tint.rgb, texColor.a * Tint.a);
}