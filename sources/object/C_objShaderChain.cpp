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

ObjShaderChain::ObjShaderChain() :
        g_geometryShader(fge::shader::gManager.getBadElement()),
        g_vertexShader(fge::shader::gManager.getBadElement()),
        g_fragmentShader(fge::shader::gManager.getBadElement()),
        g_vertexCount(3),
        g_blendMode(fge::vulkan::BlendNone),
        g_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
{}

void ObjShaderChain::first([[maybe_unused]] fge::Scene& scene)
{
    this->_drawMode = fge::Object::DrawModes::DRAW_ALWAYS_DRAWN;
}

#ifndef FGE_DEF_SERVER
FGE_OBJ_DRAW_BODY(ObjShaderChain)
{
    auto copyStates = states.copy();
    copyStates._resTransform.set(target.requestGlobalTransform(*this, states._resTransform));

    copyStates._resInstances.setFirstInstance(0);
    copyStates._resInstances.setInstancesCount(1, true);
    copyStates._resInstances.setVertexCount(this->g_vertexCount);

    copyStates._shaderVertex = this->g_vertexShader->_ptr.get();
    copyStates._shaderFragment = this->g_fragmentShader->_ptr.get();
    copyStates._shaderGeometry = this->g_geometryShader->_ptr.get();

    copyStates._blendMode = this->g_blendMode;
    copyStates._topology = this->g_topology;

    this->drawSubsidiary(target, copyStates);

    target.draw(copyStates);
}
void ObjShaderChain::drawSubsidiary([[maybe_unused]] fge::RenderTarget& target,
                                    [[maybe_unused]] fge::RenderStates& states) const
{}
#endif

void ObjShaderChain::setGeometryShader(std::string_view name)
{
    this->g_geometryShader = fge::shader::gManager.getElement(name);
}
void ObjShaderChain::setVertexShader(std::string_view name)
{
    this->g_vertexShader = fge::shader::gManager.getElement(name);
}
void ObjShaderChain::setFragmentShader(std::string_view name)
{
    this->g_fragmentShader = fge::shader::gManager.getElement(name);
}

void ObjShaderChain::setVertexCount(uint32_t count)
{
    this->g_vertexCount = count;
}
uint32_t ObjShaderChain::getVertexCount() const
{
    return this->g_vertexCount;
}

void ObjShaderChain::setBlendMode(fge::vulkan::BlendMode const& blendMode)
{
    this->g_blendMode = blendMode;
}
fge::vulkan::BlendMode const& ObjShaderChain::getBlendMode() const
{
    return this->g_blendMode;
}

void ObjShaderChain::setTopology(VkPrimitiveTopology topology)
{
    this->g_topology = topology;
}
VkPrimitiveTopology ObjShaderChain::getTopology() const
{
    return this->g_topology;
}

fge::shader::ShaderManager::DataBlockPointer ObjShaderChain::getGeometryShader() const
{
    return this->g_geometryShader;
}
fge::shader::ShaderManager::DataBlockPointer ObjShaderChain::getVertexShader() const
{
    return this->g_vertexShader;
}
fge::shader::ShaderManager::DataBlockPointer ObjShaderChain::getFragmentShader() const
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
