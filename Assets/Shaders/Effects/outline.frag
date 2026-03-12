#version 450 core
in vec2 TexCoords;
in vec4 Tint;
flat in vec4 CellUV;
out vec4 color;

uniform sampler2D image;
uniform float uTime;
uniform vec3 outlineColor;
uniform float thickness;
uniform float pulseSpeed;

void main()
{
    vec2 uvMin = CellUV.xy;
    vec2 uvMax = CellUV.xy + CellUV.zw;

    // Convert thickness from pixels to UV-space offsets
    vec2 texelSize = 1.0 / vec2(textureSize(image, 0));
    vec2 step = texelSize * thickness;

    vec4 texColor = texture(image, TexCoords);
    float alpha = texColor.a;

    // Sample neighbours, clamped to cell bounds
    float aU = texture(image, clamp(TexCoords + vec2(0.0,  step.y), uvMin, uvMax)).a;
    float aD = texture(image, clamp(TexCoords + vec2(0.0, -step.y), uvMin, uvMax)).a;
    float aL = texture(image, clamp(TexCoords + vec2(-step.x, 0.0), uvMin, uvMax)).a;
    float aR = texture(image, clamp(TexCoords + vec2( step.x, 0.0), uvMin, uvMax)).a;

    float outline = max(max(aU, aD), max(aL, aR)) - alpha;
    outline = clamp(outline, 0.0, 1.0);

    float pulse = (pulseSpeed > 0.0) ? 0.5 + 0.5 * sin(uTime * pulseSpeed) : 1.0;
    vec3 finalColor = mix(texColor.rgb * Tint.rgb, outlineColor, outline * pulse);
    float finalAlpha = max(alpha, outline) * Tint.a;

    color = vec4(finalColor, finalAlpha);
}
