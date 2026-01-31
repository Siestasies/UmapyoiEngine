#version 450 core
layout (location = 0) in vec2 vertex;
layout (location = 1) in mat4 instanceModel;
layout (location = 5) in vec3 instanceColor;

out vec3 Color;

uniform mat4 projection;

void main()
{
    Color = instanceColor;
    gl_Position = projection * instanceModel * vec4(vertex, 0.0, 1.0);
}