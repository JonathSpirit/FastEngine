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

#ifndef _FGE_C_OBJANIM_HPP_INCLUDED
#define _FGE_C_OBJANIM_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <FastEngine/C_object.hpp>
#include <FastEngine/C_animation.hpp>
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
    explicit ObjAnimation(const fge::Animation& animation, const sf::Vector2f& position=sf::Vector2f());

    FGE_OBJ_DEFAULT_COPYMETHOD(fge::ObjAnimation)

    void setAnimation(const fge::Animation& animation);
    void setTextureRect(const sf::IntRect& rectangle);

    void setColor(const sf::Color& color);

    void setPause(bool flag);
    bool isPaused() const;

    void refresh();

    void setTickDuration(const std::chrono::milliseconds& tms);
    const std::chrono::milliseconds& getTickDuration() const;

    const fge::Animation& getAnimation() const;
    fge::Animation& getAnimation();
    const sf::IntRect& getTextureRect() const;

    const sf::Color& getColor() const;

    FGE_OBJ_UPDATE_DECLARE
    FGE_OBJ_DRAW_DECLARE

    void save(nlohmann::json& jsonObject, fge::Scene* scene) override;
    void load(nlohmann::json& jsonObject, fge::Scene* scene) override;
    void pack(fge::net::Packet& pck) override;
    void unpack(fge::net::Packet& pck) override;

    const char* getClassName() const override;
    const char* getReadableClassName() const override;

    sf::FloatRect getGlobalBounds() const override;
    sf::FloatRect getLocalBounds() const override;

private:

    void updatePositions();
    void updateTexCoords();

    sf::Vertex g_vertices[4];
    fge::Animation g_animation;
    sf::IntRect g_textureRect;

    std::chrono::milliseconds g_tickDuration;
    std::chrono::milliseconds g_nextFrameTime;

    bool g_paused;
};

}//end fge

#endif // _FGE_C_OBJANIM_HPP_INCLUDED
