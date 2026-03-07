#version 450 core
in vec2 TexCoords;
in vec4 Tint;
out vec4 color;

uniform sampler2D image;

uniform float uTime;
uniform float intensity; // 0.0 = full color, 1.0 = full grayscale

void main()
{
    vec4 texColor = texture(image, TexCoords);
    vec3 tinted = texColor.rgb * Tint.rgb;
    float gray = dot(tinted, vec3(0.299, 0.587, 0.114));
    color = vec4(mix(tinted, vec3(gray), intensity), texColor.a * Tint.a);
}
