/*
 * Copyright 2026 Guillaume Guillet
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "FastEngine/graphic/shaderResources.hpp"

namespace fge::res
{

// clang-format off

const char gDefaultVertexShader[] = R"(
#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in uvec4 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;

struct InstanceData {
    mat4 modelTransform;
    mat4 viewTransform;
};

layout(set = 0, binding = 0) buffer readonly BufferInstanceData {
    InstanceData data[];
} instances;

void main()
{
    InstanceData instance = instances.data[gl_InstanceIndex];
    vec4 position = instance.viewTransform * instance.modelTransform * vec4(inPosition, 0.0, 1.0);
    position[1] *= -1.0;

    gl_Position = position;

    fragColor = vec4(float(inColor[3])/255.0, float(inColor[2])/255.0, float(inColor[1])/255.0, float(inColor[0])/255.0);
    fragTexCoord = inTexCoord;
}
)";
const unsigned int gDefaultVertexShaderSize = sizeof(gDefaultVertexShader)-1;

const char gDefaultFragmentShader[] = R"(
#version 450

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = fragColor;
}
)";
const unsigned int gDefaultFragmentShaderSize = sizeof(gDefaultFragmentShader)-1;

const char gDefaultFragmentTextureShader[] = R"(
#version 450

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D texSampler;

void main() {
    outColor = texture(texSampler, fragTexCoord) * fragColor;
}
)";
const unsigned int gDefaultFragmentTextureShaderSize = sizeof(gDefaultFragmentTextureShader)-1;

// clang-format on

} // namespace fge::res