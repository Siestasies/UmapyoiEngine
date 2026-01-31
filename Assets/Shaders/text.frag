#version 450 core
in vec2 TexCoords;
out vec4 FragColor;
layout (binding = 0) uniform sampler2D text;
uniform vec3 textColor;

void main() {
    float alpha = texture(text, TexCoords).r;
    FragColor = vec4(textColor, 1.0) * vec4(1.0, 1.0, 1.0, alpha);
}