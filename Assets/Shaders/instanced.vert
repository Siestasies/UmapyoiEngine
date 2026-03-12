#version 450 core
layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
layout (location = 1) in mat4 instanceModel; // Takes locations 1-4
layout (location = 5) in vec4 instanceUV; // <vec2 uvOffset, vec2 uvSize>
layout (location = 6) in vec4 instanceTint; // <vec3 tintColor, float alpha>

out vec2 TexCoords;    // UV-transformed coords for texture sampling
out vec2 RawTexCoords; // Raw [0,1] quad UVs for fill clipping
out vec4 Tint;

uniform mat4 projection;

void main()
{
    vec2 uv = vertex.zw;

    // Raw quad UVs - used by fill clipping in the fragment shader
    RawTexCoords = uv;

    // UV-transformed coords - used for texture sampling
    TexCoords = instanceUV.xy + uv * instanceUV.zw;

    // Pass tint to fragment shader
    Tint = instanceTint;

    gl_Position = projection * instanceModel * vec4(vertex.xy, 0.0, 1.0);
}