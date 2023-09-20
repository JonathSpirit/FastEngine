/*
 * Copyright 2023 Guillaume Guillet
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

#ifndef _FGE_C_OBJSHAPE_HPP_INCLUDED
#define _FGE_C_OBJSHAPE_HPP_INCLUDED

/*
 * Original from : https://github.com/SFML/SFML
 * Copyright (C) 2007-2022 Laurent Gomila
 *
 * Altered/Modified by Guillaume Guillet
 */

#include "FastEngine/fge_extern.hpp"
#include "C_object.hpp"
#include "FastEngine/C_texture.hpp"
#include "FastEngine/manager/shader_manager.hpp"

#define FGE_OBJSHAPE_PIPELINE_CACHE_NAME typeid(fge::ObjShape).name()
#define FGE_OBJSHAPE_INSTANCES_SHADER_VERTEX "FGE:OBJ:SHAPE:VERTEX"
#define FGE_OBJSHAPE_INSTANCES_LAYOUT "FGE:OBJ:SHAPE:LAYOUT"
#define FGE_OBJSHAPE_INDEX_FILLCOLOR 0
#define FGE_OBJSHAPE_INDEX_OUTLINECOLOR 1

namespace fge
{

class FGE_API ObjShape : public fge::Object
{
protected:
    ObjShape();
    ObjShape(ObjShape const& r);

public:
    struct InstanceData
    {
        alignas(16) glm::uvec4 _color[2];
        alignas(16) glm::vec2 _offset;
    };

    ~ObjShape() override = default;

    void setTexture(Texture const& texture, bool resetRect = false);
    void setTextureRect(fge::RectInt const& rect);

    void setFillColor(Color color, std::size_t instance = 0);
    void setOutlineColor(Color color, std::size_t instance = 0);
    void setOffset(fge::Vector2f const& offset, std::size_t instance = 0);
    void setInstancesCount(std::size_t count);
    void addInstance(Color fillColor, Color outlineColor, fge::Vector2f const& offset);
    [[nodiscard]] std::size_t getInstancesCount() const;
    void clearInstances();

    void setOutlineThickness(float thickness);

    [[nodiscard]] Texture const& getTexture() const;

    [[nodiscard]] RectInt const& getTextureRect() const;

    [[nodiscard]] Color getFillColor(std::size_t instance = 0) const;
    [[nodiscard]] Color getOutlineColor(std::size_t instance = 0) const;
    [[nodiscard]] fge::Vector2f const& getOffset(std::size_t instance = 0) const;

    [[nodiscard]] float getOutlineThickness() const;

    [[nodiscard]] virtual std::size_t getPointCount() const = 0;
    [[nodiscard]] virtual Vector2f getPoint(std::size_t index) const = 0;

    void first(fge::Scene* scene) override;

    FGE_OBJ_DRAW_DECLARE

    [[nodiscard]] fge::RectFloat getLocalBounds() const override;
    [[nodiscard]] fge::RectFloat getGlobalBounds() const override;

protected:
    void updateShape();

private:
    void updateTexCoords();
    void updateOutline();
    void resizeBuffer(std::size_t size) const;

    inline InstanceData* retrieveInstance(std::size_t index) const;

    fge::Texture g_texture;
    fge::RectInt g_textureRect;

    float g_outlineThickness;

    fge::vulkan::VertexBuffer g_vertices;
    fge::vulkan::VertexBuffer g_outlineVertices;

    mutable std::size_t g_instancesCount;
    mutable std::size_t g_instancesCapacity;
    mutable fge::vulkan::UniformBuffer g_instances;
    mutable fge::vulkan::DescriptorSet g_descriptorSet;

    fge::RectFloat g_insideBounds;
    fge::RectFloat g_bounds;
};

} // namespace fge


#endif // _FGE_C_OBJSHAPE_HPP_INCLUDED
