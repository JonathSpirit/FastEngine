/*
 * Copyright 2022 Guillaume Guillet
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

#ifndef _FGE_C_OBJSPRITEBATCHES_HPP_INCLUDED
#define _FGE_C_OBJSPRITEBATCHES_HPP_INCLUDED

#include "FastEngine/fastengine_extern.hpp"
#include "FastEngine/extra/extra_function.hpp"
#include "C_object.hpp"
#include "FastEngine/C_texture.hpp"

#define FGE_OBJSPRITEBATCHES_CLASSNAME "FGE:OBJ:SPRITEBATCHES"

namespace fge
{

class FGE_API ObjSpriteBatches : public fge::Object
{
public:
    ObjSpriteBatches();
    ObjSpriteBatches(const ObjSpriteBatches& r);
    explicit ObjSpriteBatches(fge::Texture texture);

    FGE_OBJ_DEFAULT_COPYMETHOD(fge::ObjSpriteBatches)

    void setTexture(fge::Texture texture);

    fge::Transformable& addSprite(const fge::RectInt& rectangle);
    void resize(std::size_t size);
    void setTextureRect(std::size_t index, const fge::RectInt& rectangle);
    void setColor(std::size_t index, const fge::Color& color);

    const fge::Texture& getTexture() const;

    [[nodiscard]] std::optional<fge::RectInt> getTextureRect(std::size_t index) const;
    [[nodiscard]] std::optional<fge::Color> getColor(std::size_t index) const;

    FGE_OBJ_DRAW_DECLARE

    void save(nlohmann::json& jsonObject, fge::Scene* scene) override;
    void load(nlohmann::json& jsonObject, fge::Scene* scene) override;
    void pack(fge::net::Packet& pck) override;
    void unpack(fge::net::Packet& pck) override;

    const char* getClassName() const override;
    const char* getReadableClassName() const override;

    [[nodiscard]] fge::RectFloat getGlobalBounds() const override;
    [[nodiscard]] std::optional<fge::RectFloat> getGlobalBounds(std::size_t index) const;
    [[nodiscard]] fge::RectFloat getLocalBounds() const override;
    [[nodiscard]] std::optional<fge::RectFloat> getLocalBounds(std::size_t index) const;

private:
    void updatePositions(std::size_t index) const;
    void updateTexCoords(std::size_t index) const;
    void updateBuffers() const;

    struct TransformData ///TODO: Avoid duplicate with fge::Transform
    {
        alignas(16) glm::mat4 _modelTransform{1.0f};
        alignas(16) glm::mat4 _viewTransform{1.0f};
    };

    fge::Texture g_texture;

    mutable fge::vulkan::DescriptorSet g_descriptorSet;

    mutable std::vector<fge::Transformable> g_instancesTransformable;
    mutable std::unique_ptr<TransformData[], fge::AlignedDeleter> g_instancesTransformData;
    mutable fge::vulkan::UniformBuffer g_instancesTransform;
    mutable fge::vulkan::VertexBuffer g_instancesVertices;
    mutable std::vector<fge::RectInt> g_instancesTextureRect;

    std::size_t g_spriteCount;

    mutable bool g_needBuffersUpdate;
};

} // namespace fge

#endif // _FGE_C_OBJSPRITEBATCHES_HPP_INCLUDED
