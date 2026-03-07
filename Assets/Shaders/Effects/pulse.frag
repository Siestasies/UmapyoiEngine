#version 450 core
in vec2 TexCoords;
in vec4 Tint;
out vec4 color;

uniform sampler2D image;
uniform float uTime;
uniform float speed;      // pulse speed, e.g. 3.0
uniform float minGlow;    // minimum brightness, e.g. 0.8
uniform float maxGlow;    // maximum brightness, e.g. 1.5
uniform vec3 glowColor;   // additive glow tint, e.g. vec3(0.2, 0.1, 0.0)

void main()
{
    vec4 texColor = texture(image, TexCoords);
    vec3 tinted = texColor.rgb * Tint.rgb;

    float pulse = mix(minGlow, maxGlow, 0.5 + 0.5 * sin(uTime * speed));
    vec3 glow = glowColor * (pulse - 1.0);

    color = vec4(tinted * pulse + glow, texColor.a * Tint.a);
}
