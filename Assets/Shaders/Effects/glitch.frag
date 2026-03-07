#version 450 core
in vec2 TexCoords;
in vec4 Tint;
flat in vec4 CellUV;
out vec4 color;

uniform sampler2D image;
uniform float uTime;
uniform float intensity;   // glitch strength, e.g. 0.05
uniform float speed;       // how fast glitches change, e.g. 10.0
uniform float blockSize;   // horizontal slice height, e.g. 0.05

void main()
{
    vec2 uvMin = CellUV.xy;
    vec2 uvMax = CellUV.xy + CellUV.zw;

    // Random based on time and vertical position
    float line = floor(TexCoords.y / blockSize);
    float noise = fract(sin(line * 43758.5453 + floor(uTime * speed)) * 12345.6789);

    vec2 uv = TexCoords;
    // Horizontal displacement on random lines
    if (noise > 0.6)
        uv.x += (noise - 0.6) * intensity * sign(sin(uTime * 17.0));

    uv = clamp(uv, uvMin, uvMax);

    // RGB channel split
    float r = texture(image, clamp(uv + vec2(intensity * 0.3 * noise, 0.0), uvMin, uvMax)).r;
    float g = texture(image, uv).g;
    float b = texture(image, clamp(uv - vec2(intensity * 0.3 * noise, 0.0), uvMin, uvMax)).b;
    float a = texture(image, uv).a;

    color = vec4(vec3(r, g, b) * Tint.rgb, a * Tint.a);
}
