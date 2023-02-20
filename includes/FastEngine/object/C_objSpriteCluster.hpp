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

#ifndef _FGE_C_OBJSPRITECLUSTER_HPP_INCLUDED
#define _FGE_C_OBJSPRITECLUSTER_HPP_INCLUDED

#include "FastEngine/fastengine_extern.hpp"
#include "C_object.hpp"
#include "FastEngine/C_texture.hpp"
#include "FastEngine/extra/extra_function.hpp"

#define FGE_OBJSPRITECLUSTER_CLASSNAME "FGE:OBJ:SPRITECLUSTER"

namespace fge
{

class FGE_API ObjSpriteCluster : public fge::Object
{
public:
    ObjSpriteCluster();
    ObjSpriteCluster(const ObjSpriteCluster& r);
    explicit ObjSpriteCluster(fge::Texture texture);

    FGE_OBJ_DEFAULT_COPYMETHOD(fge::ObjSpriteCluster)

    void setTexture(fge::Texture texture);

    void clear();
    void addSprite(const fge::RectInt& rectangle, const fge::Vector2f& offset = {0.0f, 0.0f});
    void resize(std::size_t size);
    void setTextureRect(std::size_t index, const fge::RectInt& rectangle);
    void setColor(std::size_t index, const fge::Color& color);
    void setOffset(std::size_t index, const fge::Vector2f& offset);

    const fge::Texture& getTexture() const;

    [[nodiscard]] std::optional<fge::RectInt> getTextureRect(std::size_t index) const;
    [[nodiscard]] std::optional<fge::Color> getColor(std::size_t index) const;
    [[nodiscard]] std::optional<fge::Vector2f> getOffset(std::size_t index) const;

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

    struct InstanceData
    {
        InstanceData() = default;
        explicit InstanceData(const fge::RectInt& textureRect, const fge::Vector2f& offset) :
                _offset(offset),
                _textureRect(textureRect)
        {}

        fge::Vector2f _offset;
        fge::RectInt _textureRect;
    };

    fge::Texture g_texture;

    std::vector<InstanceData> g_instancesData;
    mutable fge::vulkan::VertexBuffer g_instancesVertices;

    mutable bool g_needBuffersUpdate;
};

} // namespace fge

#endif // _FGE_C_OBJSPRITECLUSTER_HPP_INCLUDED
