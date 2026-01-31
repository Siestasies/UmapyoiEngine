#version 450 core
in vec2 TexCoords;
in vec4 Tint;
out vec4 color;

uniform sampler2D image;

void main()
{
    vec4 texColor = texture(image, TexCoords);
    // Apply tint to RGB and alpha separately
    color = vec4(texColor.rgb * Tint.rgb, texColor.a * Tint.a);
}