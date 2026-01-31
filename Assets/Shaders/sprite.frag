#version 450 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D image;
uniform vec3 debugColor;
uniform int useDebugColor;
uniform vec3 tintColor;
uniform float alpha;

void main()
{
    if (useDebugColor == 1) {
        color = vec4(debugColor, 1.0);
    } else {
        vec4 texColor = texture(image, TexCoords);
        color = vec4(texColor.rgb * tintColor, texColor.a * alpha);
    }
}