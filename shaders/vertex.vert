#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 modelTransform;
    mat4 viewTransform;
} uboTransform;

void main()
{
    vec4 position = uboTransform.viewTransform * uboTransform.modelTransform * vec4(inPosition, 0.0, 1.0);
    position[1] *= -1.0;

    gl_Position = position;
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}
