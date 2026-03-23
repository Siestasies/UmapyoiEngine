#version 450 core
layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>

out vec2 TexCoords;

void main()
{
    // Flip V to match FBO orientation (OpenGL FBO has (0,0) at bottom-left)
    TexCoords = vec2(vertex.z, 1.0 - vertex.w);
    // Fullscreen quad: vertex.xy is [-0.5, 0.5], scale to [-1, 1] for NDC
    gl_Position = vec4(vertex.xy * 2.0, 0.0, 1.0);
}
