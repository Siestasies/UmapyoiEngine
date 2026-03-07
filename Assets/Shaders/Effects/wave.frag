#version 450 core
in vec2 TexCoords;
in vec4 Tint;
flat in vec4 CellUV; // xy = uvOffset, zw = uvSize
out vec4 color;

uniform sampler2D image;
uniform float uTime;
uniform float amplitude;  // distortion strength, e.g. 0.02
uniform float frequency;  // wave density, e.g. 10.0
uniform float speed;      // animation speed, e.g. 3.0

void main()
{
    vec2 uv = TexCoords;
    uv.x += sin(uv.y * frequency + uTime * speed) * amplitude;
    uv.y += cos(uv.x * frequency + uTime * speed) * amplitude * 0.5;

    vec2 uvMin = CellUV.xy;
    vec2 uvMax = CellUV.xy + CellUV.zw;
    uv = clamp(uv, uvMin, uvMax);

    vec4 texColor = texture(image, uv);
    color = vec4(texColor.rgb * Tint.rgb, texColor.a * Tint.a);
}
