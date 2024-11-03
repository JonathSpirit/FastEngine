#version 450

//layout(location = 0) in vec2 inPosition;
//layout(location = 1) in uvec4 inColor;
//layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;

const vec2 inPosition[3] = vec2[3](vec2(0, 0), vec2(20, 0), vec2(20, 20));
const uvec4 inColor = uvec4(255, 0, 0, 255);

struct InstanceData {
    mat4 modelTransform;
    mat4 viewTransform;
};

layout(set = 0, binding = 0) buffer BufferInstanceData {
    InstanceData data[];
} instances;

void main()
{
    InstanceData instance = instances.data[gl_InstanceIndex];
    vec4 position = instance.viewTransform * instance.modelTransform * vec4(inPosition[gl_VertexIndex], 0.0, 1.0);
    position[1] *= -1.0;

    gl_Position = position;

    fragColor = vec4(float(inColor[3])/255.0, float(inColor[2])/255.0, float(inColor[1])/255.0, float(inColor[0])/255.0);
    fragTexCoord = vec2(0.0, 0.0);
}
