#version 450 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D lightMap;

void main()
{
    vec3 light = clamp(texture(lightMap, TexCoords).rgb, 0.0, 2.0);
    FragColor = vec4(light, 1.0);
}
