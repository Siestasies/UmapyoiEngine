#version 450 core
in vec2 TexCoords;
out vec4 FragColor;

uniform vec3  lightColor;
uniform float intensity;
uniform float innerRadius; // fraction of radius at full brightness (0-1)

void main()
{
    // TexCoords are 0-1 across the quad; center is (0.5, 0.5)
    vec2 centered = TexCoords - vec2(0.5);
    float dist = length(centered) * 2.0; // 0 at center, 1 at edge

    // Smooth falloff from innerRadius to 1.0
    float attenuation = 1.0 - smoothstep(innerRadius, 1.0, dist);

    FragColor = vec4(lightColor * intensity * attenuation, 1.0);
}
