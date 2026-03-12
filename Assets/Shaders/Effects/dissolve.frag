#version 450 core
in vec2 TexCoords;
in vec4 Tint;
out vec4 color;

uniform sampler2D image;
uniform float uTime;
uniform float threshold;  // dissolve progress 0.0 (solid) to 1.0 (gone)
uniform float edgeWidth;  // glow edge width, e.g. 0.05
uniform vec3 edgeColor;   // dissolve edge color, e.g. vec3(1.0, 0.5, 0.0)

void main()
{
    vec4 texColor = texture(image, TexCoords);

    // Procedural noise pattern based on UV
    float noise = fract(sin(dot(TexCoords, vec2(12.9898, 78.233))) * 43758.5453);

    if (noise < threshold)
        discard;

    // Edge glow
    float edge = smoothstep(threshold, threshold + edgeWidth, noise);
    vec3 finalColor = mix(edgeColor, texColor.rgb * Tint.rgb, edge);

    color = vec4(finalColor, texColor.a * Tint.a);
}
