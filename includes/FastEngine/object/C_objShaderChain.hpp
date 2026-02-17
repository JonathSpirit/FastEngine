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

#ifndef _FGE_C_OBJSHADERCHAIN_HPP_INCLUDED
#define _FGE_C_OBJSHADERCHAIN_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "C_object.hpp"
#include "FastEngine/accessor/C_shader.hpp"

#define FGE_OBJSHADERCHAIN_CLASSNAME "FGE:OBJ:SHADERCHAIN"

namespace fge
{

class FGE_API ObjShaderChain : public fge::Object
{
public:
    ObjShaderChain();

    FGE_OBJ_DEFAULT_COPYMETHOD(fge::ObjShaderChain)

    FGE_OBJ_DRAW_DECLARE

    void first(fge::Scene& scene) override;

    void setGeometryShader(fge::Shader shader);
    void setVertexShader(fge::Shader shader);
    void setFragmentShader(fge::Shader shader);

    void setVertexCount(uint32_t count);
    [[nodiscard]] uint32_t getVertexCount() const;

    void setBlendMode(fge::vulkan::BlendMode const& blendMode);
    [[nodiscard]] fge::vulkan::BlendMode const& getBlendMode() const;

    void setTopology(VkPrimitiveTopology topology);
    [[nodiscard]] VkPrimitiveTopology getTopology() const;

    [[nodiscard]] fge::Shader const& getGeometryShader() const;
    [[nodiscard]] fge::Shader const& getVertexShader() const;
    [[nodiscard]] fge::Shader const& getFragmentShader() const;

    char const* getClassName() const override;
    char const* getReadableClassName() const override;

    fge::RectFloat getGlobalBounds() const override;
    fge::RectFloat getLocalBounds() const override;

#ifndef FGE_DEF_SERVER
protected:
    virtual void drawSubsidiary(fge::RenderTarget& target, fge::RenderStates& states) const;
#endif

private:
    fge::Shader g_geometryShader;
    fge::Shader g_vertexShader;
    fge::Shader g_fragmentShader;
    uint32_t g_vertexCount;
    fge::vulkan::BlendMode g_blendMode;
    VkPrimitiveTopology g_topology;
};

} // namespace fge

#endif // _FGE_C_OBJSHADERCHAIN_HPP_INCLUDED
