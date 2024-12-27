#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in uvec4 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) flat out uint fragTexIndex;

struct InstanceData {
    mat4 transform;
    uint textureIndex;
};

layout(set = 0, binding = 0) buffer readonly BufferInstanceData {
    InstanceData data[];
} instances;

void main()
{
    InstanceData instance = instances.data[gl_InstanceIndex+1];
    vec4 position = instances.data[0].transform * instance.transform * vec4(inPosition, 0.0, 1.0);
    position[1] *= -1.0;

    gl_Position = position;

    fragColor = vec4(float(inColor[3])/255.0, float(inColor[2])/255.0, float(inColor[1])/255.0, float(inColor[0])/255.0);
    fragTexCoord = inTexCoord;
    fragTexIndex = instance.textureIndex;
}
