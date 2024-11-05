#version 450

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

layout(location = 0) in vec4 vertColor[];
layout(location = 1) in vec2 vertTexCoord[];

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main()
{
    for (int i = 0; i < 3; ++i)
    {
        gl_Position = gl_in[i].gl_Position;
        fragColor = vertColor[i];
        fragTexCoord = vertTexCoord[i];
        EmitVertex();
    }
    EndPrimitive();
}
