#version 450 core
layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
layout (location = 1) in mat4 instanceModel; // Takes locations 1-4
layout (location = 5) in vec4 instanceUV; // <vec2 uvOffset, vec2 uvSize>
layout (location = 6) in vec4 instanceTint; // <vec3 tintColor, float alpha>

out vec2 TexCoords;
out vec4 Tint;
flat out vec4 CellUV;

uniform mat4 projection;

void main()
{
    // Apply UV transformation
    vec2 uv = vertex.zw;
    TexCoords = instanceUV.xy + uv * instanceUV.zw;
    
    // Pass tint to fragment shader
    Tint = instanceTint;

    // Pass cell UV bounds for effect shaders
    CellUV = instanceUV;
    
    gl_Position = projection * instanceModel * vec4(vertex.xy, 0.0, 1.0);
}
