#version 450 core
in vec2 TexCoords;
in vec4 Tint;
out vec4 color;

uniform sampler2D image;

// Add your custom uniforms here, e.g.:
// uniform float intensity;

uniform float intensity;    // 0.0 = normal, 1.0 = full effect
uniform vec3 overlayColor;  // color to blend in

void main()
{
    vec4 texColor = texture(image, TexCoords);
    vec3 blended = mix(texColor.rgb * Tint.rgb, overlayColor, intensity);
    color = vec4(blended, texColor.a * Tint.a);
}
