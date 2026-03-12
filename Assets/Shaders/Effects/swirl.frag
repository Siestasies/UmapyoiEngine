#version 450 core
in vec2 TexCoords;
in vec4 Tint;
flat in vec4 CellUV;
out vec4 color;

uniform sampler2D image;
uniform float uTime;
uniform float radius;   // swirl radius, e.g. 0.5
uniform float strength; // swirl angle, e.g. 2.0
uniform float speed;    // animation speed, e.g. 1.0

void main()
{
    // Compute center of current cell in UV space
    vec2 cellCenter = CellUV.xy + CellUV.zw * 0.5;

    vec2 uv = TexCoords - cellCenter;
    float dist = length(uv / CellUV.zw);
    float angle = strength * sin(uTime * speed) * max(0.0, 1.0 - dist / radius);
    float s = sin(angle);
    float c = cos(angle);
    uv = vec2(uv.x * c - uv.y * s, uv.x * s + uv.y * c) + cellCenter;

    vec2 uvMin = CellUV.xy;
    vec2 uvMax = CellUV.xy + CellUV.zw;
    uv = clamp(uv, uvMin, uvMax);

    vec4 texColor = texture(image, uv);
    color = vec4(texColor.rgb * Tint.rgb, texColor.a * Tint.a);
}
