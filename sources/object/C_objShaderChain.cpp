/*
 * Copyright 2024 Guillaume Guillet
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

#include "FastEngine/object/C_objShaderChain.hpp"

namespace fge
{

namespace
{

struct CustomData
{
    fge::vulkan::Shader const* _geometryShader;
    fge::vulkan::Shader const* _vertexShader;
    fge::vulkan::Shader const* _fragmentShader;
};

void GraphicPipeline_constructor([[maybe_unused]] fge::vulkan::Context const& context,
                                 fge::RenderTarget::GraphicPipelineKey const& key,
                                 fge::vulkan::GraphicPipeline* graphicPipeline,
                                 void* customData)
{
    auto const* custom = static_cast<CustomData*>(customData);

    graphicPipeline->setShader(*custom->_geometryShader);
    graphicPipeline->setShader(*custom->_vertexShader);
    graphicPipeline->setShader(*custom->_fragmentShader);
    graphicPipeline->setBlendMode(key._blendMode);
    graphicPipeline->setPrimitiveTopology(key._topology);
}

} // namespace

ObjShaderChain::ObjShaderChain() :
        g_geometryShader(fge::shader::GetBadShader()),
        g_vertexShader(fge::shader::GetBadShader()),
        g_fragmentShader(fge::shader::GetBadShader())
{}

#ifndef FGE_DEF_SERVER
FGE_OBJ_DRAW_BODY(ObjShaderChain)
{
    auto copyStates = states.copy();

    RenderTarget::GraphicPipelineKey const key{VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, fge::vulkan::BlendNone, 0};
    CustomData customData{&this->g_geometryShader->_shader, &this->g_vertexShader->_shader,
                          &this->g_fragmentShader->_shader};

    auto* graphicPipeline =
            target.getGraphicPipeline(FGE_OBJSHADERCHAIN_CLASSNAME, key, GraphicPipeline_constructor, &customData);

    target.draw(copyStates, graphicPipeline);
}
#endif

void ObjShaderChain::setGeometryShader(std::string_view name)
{
    this->g_geometryShader = fge::shader::GetShader(name);
}
void ObjShaderChain::setVertexShader(std::string_view name)
{
    this->g_vertexShader = fge::shader::GetShader(name);
}
void ObjShaderChain::setFragmentShader(std::string_view name)
{
    this->g_fragmentShader = fge::shader::GetShader(name);
}

fge::shader::ShaderDataPtr ObjShaderChain::getGeometryShader() const
{
    return this->g_geometryShader;
}
fge::shader::ShaderDataPtr ObjShaderChain::getVertexShader() const
{
    return this->g_vertexShader;
}
fge::shader::ShaderDataPtr ObjShaderChain::getFragmentShader() const
{
    return this->g_fragmentShader;
}

char const* ObjShaderChain::getClassName() const
{
    return FGE_OBJSHADERCHAIN_CLASSNAME;
}
char const* ObjShaderChain::getReadableClassName() const
{
    return "shader chain";
}

fge::RectFloat ObjShaderChain::getGlobalBounds() const
{
    return this->getTransform() * this->getLocalBounds();
}
fge::RectFloat ObjShaderChain::getLocalBounds() const
{
    return {{0.f, 0.f}, {1.0f, 1.0f}};
}

} // namespace fge
