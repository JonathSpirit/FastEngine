/*
 * Copyright 2025 Guillaume Guillet
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

#ifndef _FGE_C_OBJANIM_HPP_INCLUDED
#define _FGE_C_OBJANIM_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "C_object.hpp"
#include "FastEngine/accessor/C_animation.hpp"
#include <chrono>
#include <string>

#define FGE_OBJANIM_DEFAULT_TICKDURATION_MS 10
#define FGE_OBJANIM_CLASSNAME "FGE:OBJ:ANIM"

namespace fge
{

class FGE_API ObjAnimation : public fge::Object
{
public:
    ObjAnimation();
    explicit ObjAnimation(fge::Animation const& animation, fge::Vector2f const& position = fge::Vector2f());

    FGE_OBJ_DEFAULT_COPYMETHOD(fge::ObjAnimation)

    void setAnimation(fge::Animation const& animation);
    void setTextureRect(fge::RectInt const& rectangle);

    void setColor(fge::Color const& color);

    void setPause(bool flag);
    bool isPaused() const;

    void refresh();

    void setTickDuration(std::chrono::milliseconds const& tms);
    std::chrono::microseconds const& getTickDuration() const;

    fge::Animation const& getAnimation() const;
    fge::Animation& getAnimation();
    fge::RectInt const& getTextureRect() const;

    fge::Color getColor() const;

    FGE_OBJ_UPDATE_DECLARE
    FGE_OBJ_DRAW_DECLARE

    void save(nlohmann::json& jsonObject) override;
    void load(nlohmann::json& jsonObject, std::filesystem::path const& filePath) override;
    void pack(fge::net::Packet& pck) override;
    void unpack(fge::net::Packet const& pck) override;

    char const* getClassName() const override;
    char const* getReadableClassName() const override;

    fge::RectFloat getGlobalBounds() const override;
    fge::RectFloat getLocalBounds() const override;

private:
    void updatePositions();
    void updateTexCoords();

    fge::vulkan::VertexBuffer g_vertices;
    fge::Animation g_animation;
    fge::RectInt g_textureRect;

    std::chrono::microseconds g_tickDuration;
    std::chrono::microseconds g_nextFrameTime;

    bool g_paused;
};

} // namespace fge

#endif // _FGE_C_OBJANIM_HPP_INCLUDED
