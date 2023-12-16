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

#ifndef _FGE_C_OBJSPRITEBATCHES_HPP_INCLUDED
#define _FGE_C_OBJSPRITEBATCHES_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "C_object.hpp"
#include "FastEngine/C_texture.hpp"
#include "FastEngine/extra/extra_function.hpp"

#define FGE_OBJSPRITEBATCHES_CLASSNAME "FGE:OBJ:SPRITEBATCHES"

#define FGE_OBJSPRITEBATCHES_PIPELINE_CACHE_NAME FGE_OBJSPRITEBATCHES_CLASSNAME
#define FGE_OBJSPRITEBATCHES_ID 0
#define FGE_OBJSPRITEBATCHES_ID_TEXTURE 1
#define FGE_OBJSPRITEBATCHES_SHADER_VERTEX "FGE:OBJ:SPRITEBATCHES:VERTEX"
#define FGE_OBJSPRITEBATCHES_SHADER_FRAGMENT "FGE:OBJ:SPRITEBATCHES:FRAGMENT"
#define FGE_OBJSPRITEBATCHES_LAYOUT "FGE:OBJ:SPRITEBATCHES:LAYOUT"
#define FGE_OBJSPRITEBATCHES_LAYOUT_TEXTURES "FGE:OBJ:SPRITEBATCHES:LAYOUTTEXTURES"

#define FGE_OBJSPRITEBATCHES_MAXIMUM_TEXTURES FGE_MULTIUSE_POOL_MAX_COMBINED_IMAGE_SAMPLER

namespace fge
{

class FGE_API ObjSpriteBatches : public fge::Object
{
public:
    ObjSpriteBatches();
    ObjSpriteBatches(ObjSpriteBatches const& r);
    explicit ObjSpriteBatches(fge::Texture texture);

    FGE_OBJ_DEFAULT_COPYMETHOD(fge::ObjSpriteBatches)

    void addTexture(fge::Texture texture);
    void setTexture(std::size_t index, fge::Texture texture);
    void setTexture(fge::Texture texture);
    fge::Texture const& getTexture(std::size_t index) const;
    std::size_t getTextureCount() const;
    void clearTexture();

    void clear();
    fge::Transformable& addSprite(fge::RectInt const& rectangle, uint32_t textureIndex = 0);
    void resize(std::size_t size);
    void setTextureRect(std::size_t index, fge::RectInt const& rectangle);
    void setColor(std::size_t index, fge::Color const& color);
    void setSpriteTexture(std::size_t spriteIndex, uint32_t textureIndex);
    [[nodiscard]] std::size_t getSpriteCount() const;

    [[nodiscard]] std::optional<fge::RectInt> getTextureRect(std::size_t index) const;
    [[nodiscard]] std::optional<fge::Color> getColor(std::size_t index) const;

    [[nodiscard]] fge::Transformable* getTransformable(std::size_t index);
    [[nodiscard]] fge::Transformable const* getTransformable(std::size_t index) const;

    FGE_OBJ_DRAW_DECLARE

    void save(nlohmann::json& jsonObject, fge::Scene* scene) override;
    void load(nlohmann::json& jsonObject, fge::Scene* scene) override;
    void pack(fge::net::Packet& pck) override;
    void unpack(fge::net::Packet const& pck) override;

    char const* getClassName() const override;
    char const* getReadableClassName() const override;

    [[nodiscard]] fge::RectFloat getGlobalBounds() const override;
    [[nodiscard]] std::optional<fge::RectFloat> getGlobalBounds(std::size_t index) const;
    [[nodiscard]] fge::RectFloat getLocalBounds() const override;
    [[nodiscard]] std::optional<fge::RectFloat> getLocalBounds(std::size_t index) const;

private:
    void updatePositions(std::size_t index);
    void updateTexCoords(std::size_t index);
    void updateBuffers() const;
    void updateTextures(bool sizeHasChanged);

    struct InstanceData
    {
        InstanceData() = default;
        explicit InstanceData(fge::RectInt const& textureRect, glm::uint textureIndex) :
                _textureRect(textureRect),
                _textureIndex(textureIndex)
        {}

        fge::Transformable _transformable;
        fge::RectInt _textureRect;
        glm::uint _textureIndex{0};
    };
    struct InstanceDataBuffer
    {
        alignas(16) glm::mat4 _transform;
        alignas(16) glm::uint _textureIndex{0};
    };

    std::vector<fge::Texture> g_textures;

    std::vector<InstanceData> g_instancesData;
    mutable std::size_t g_instancesTransformDataCapacity;
    mutable fge::vulkan::UniformBuffer g_instancesTransform;
    mutable fge::vulkan::DescriptorSet g_descriptorSets[2];
    fge::vulkan::VertexBuffer g_instancesVertices;

    mutable bool g_needBuffersUpdate;
};

} // namespace fge

#endif // _FGE_C_OBJSPRITEBATCHES_HPP_INCLUDED
