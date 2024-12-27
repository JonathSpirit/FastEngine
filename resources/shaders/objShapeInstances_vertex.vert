#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in uvec4 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;

struct TransformsData {
    mat4 modelTransform;
    mat4 viewTransform;
};
layout(set = 0, binding = 0) buffer readonly GlobalTransformsData {
    TransformsData data[];
} globalTransforms;

struct InstanceData {
    uvec4 color[2];
    vec2 offset;
};
layout(set = 1, binding = 0) buffer readonly BufferInstanceData {
    InstanceData data[];
} instances;

layout(push_constant) uniform ConstantData {
    uint colorIndex;
    uint transformsIndex;
} constantData;

void main()
{
    InstanceData instance = instances.data[gl_InstanceIndex];
    TransformsData transforms = globalTransforms.data[constantData.transformsIndex];
    vec4 position = transforms.viewTransform * transforms.modelTransform * vec4(inPosition+instance.offset, 0.0, 1.0);
    position[1] *= -1.0;

    gl_Position = position;

    fragColor = vec4(float(instance.color[constantData.colorIndex][0])/255.0,
                     float(instance.color[constantData.colorIndex][1])/255.0,
                     float(instance.color[constantData.colorIndex][2])/255.0,
                     float(instance.color[constantData.colorIndex][3])/255.0);
    fragTexCoord = inTexCoord;
}
