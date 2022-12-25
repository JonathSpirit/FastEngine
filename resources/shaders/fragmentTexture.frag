#version 450

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 1) uniform sampler2D texSampler;

void main() {
    vec2 texCoord = vec2(1.0-fragTexCoord[0], fragTexCoord[1]);

    outColor = texture(texSampler, texCoord);
}
