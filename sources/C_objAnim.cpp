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

#include "FastEngine/C_objAnim.hpp"

namespace fge
{

ObjAnimation::ObjAnimation() :
    g_tickDuration(FGE_OBJANIM_DEFAULT_TICKDURATION_MS),

    g_paused(false)
{
    this->setTextureRect(this->g_animation);
}
ObjAnimation::ObjAnimation(const fge::Animation& animation, const sf::Vector2f& position) :
    g_animation(animation),
    g_tickDuration(FGE_OBJANIM_DEFAULT_TICKDURATION_MS),

    g_paused(false)
{
    this->setPosition(position);
    this->setTextureRect(this->g_animation);
}

void ObjAnimation::setAnimation(const fge::Animation& animation)
{
    this->g_animation = animation;
    this->setTextureRect(this->g_animation);
}
void ObjAnimation::setTextureRect(const sf::IntRect& rectangle)
{
    if (rectangle != this->g_textureRect)
    {
        this->g_textureRect = rectangle;
        this->updatePositions();
        this->updateTexCoords();
    }
}

void ObjAnimation::setColor(const sf::Color& color)
{
    this->g_vertices[0].color = color;
    this->g_vertices[1].color = color;
    this->g_vertices[2].color = color;
    this->g_vertices[3].color = color;
}

void ObjAnimation::setPause(bool flag)
{
    this->g_paused = flag;
}
bool ObjAnimation::isPaused() const
{
    return this->g_paused;
}

void ObjAnimation::refresh()
{
    this->g_nextFrameTime = std::chrono::milliseconds{0};
    this->setTextureRect(this->g_animation);
}

void ObjAnimation::setTickDuration(const std::chrono::milliseconds& tms)
{
    this->g_tickDuration = tms;
}
const std::chrono::milliseconds& ObjAnimation::getTickDuration() const
{
    return this->g_tickDuration;
}

const fge::Animation& ObjAnimation::getAnimation() const
{
    return this->g_animation;
}
fge::Animation& ObjAnimation::getAnimation()
{
    return this->g_animation;
}
const sf::IntRect& ObjAnimation::getTextureRect() const
{
    return this->g_textureRect;
}

const sf::Color& ObjAnimation::getColor() const
{
    return this->g_vertices[0].color;
}

void ObjAnimation::update(sf::RenderWindow& screen, fge::Event& event, const std::chrono::milliseconds& deltaTime, fge::Scene* scene_ptr)
{
    if (!this->g_paused)
    {
        fge::anim::AnimationFrame* frame = this->g_animation.getFrame();
        if ( frame != nullptr )
        {
            this->g_nextFrameTime += deltaTime;
            if ( this->g_nextFrameTime >= std::chrono::milliseconds{this->g_tickDuration*frame->_ticks} )
            {
                this->g_animation.nextFrame();
                this->setTextureRect(this->g_animation);
                this->g_nextFrameTime = std::chrono::milliseconds{0};
            }
        }
        else
        {
            this->g_animation.setFrame(0);
            this->g_nextFrameTime = std::chrono::milliseconds{0};
        }
    }
}
void ObjAnimation::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    states.transform *= this->getTransform();
    states.texture = static_cast<const sf::Texture*>(this->g_animation);
    target.draw(this->g_vertices, 4, sf::TriangleStrip, states);
}

void ObjAnimation::save(nlohmann::json& jsonObject, fge::Scene* scene_ptr)
{
    fge::Object::save(jsonObject, scene_ptr);

    jsonObject["color"] = this->g_vertices[0].color.toInteger();
    jsonObject["animation"] = this->g_animation;
    jsonObject["animationGroup"] = this->g_animation.getGroupIndex();
    jsonObject["animationFrame"] = this->g_animation.getFrameIndex();
    jsonObject["animationLoop"] = this->g_animation.isLoop();
    jsonObject["animationReverse"] = this->g_animation.isReverse();
    jsonObject["tickDuration"] = static_cast<uint16_t>(this->g_tickDuration.count());
}
void ObjAnimation::load(nlohmann::json& jsonObject, fge::Scene* scene_ptr)
{
    fge::Object::load(jsonObject, scene_ptr);

    this->setColor( sf::Color( jsonObject.value<uint32_t>("color", 0) ) );
    this->g_animation = jsonObject.value<std::string>("animation", FGE_ANIM_BAD);
    this->g_animation.setGroup( jsonObject.value<std::size_t>("animationGroup", 0) );
    this->g_animation.setFrame( jsonObject.value<std::size_t>("animationFrame", 0) );
    this->g_animation.setLoop( jsonObject.value<bool>("animationLoop", false) );
    this->g_animation.setReverse( jsonObject.value<bool>("animationReverse", false) );
    this->g_tickDuration = std::chrono::milliseconds( jsonObject.value<uint16_t>("tickDuration", FGE_OBJANIM_DEFAULT_TICKDURATION_MS) );

    this->setTextureRect(this->g_animation);
}
void ObjAnimation::pack(fge::net::Packet& pck)
{
    fge::Object::pack(pck);

    pck << this->g_vertices[0].color << this->g_animation;
    pck << static_cast<uint32_t>(this->g_animation.getGroupIndex()) << static_cast<uint32_t>(this->g_animation.getFrameIndex());
    pck << this->g_animation.isLoop() << this->g_animation.isReverse();
    pck << static_cast<uint16_t>(this->g_tickDuration.count());
}
void ObjAnimation::unpack(fge::net::Packet& pck)
{
    fge::Object::unpack(pck);

    sf::Color color;
    pck >> color >> this->g_animation;
    this->setColor(color);
    uint32_t group=0, frame=0;
    bool loop=false, reverse=false;
    pck >> group >> frame >> loop >> reverse;
    this->g_animation.setGroup(group);
    this->g_animation.setFrame(frame);
    this->g_animation.setLoop(loop);
    this->g_animation.setReverse(reverse);

    uint16_t tmpTick = FGE_OBJANIM_DEFAULT_TICKDURATION_MS;
    pck >> tmpTick;
    this->g_tickDuration = std::chrono::milliseconds(tmpTick);

    this->setTextureRect(this->g_animation);
}

std::string ObjAnimation::getClassName() const
{
    return FGE_OBJANIM_CLASSNAME;
}
std::string ObjAnimation::getReadableClassName() const
{
    return "animation";
}

sf::FloatRect ObjAnimation::getGlobalBounds() const
{
    return this->getTransform().transformRect(this->getLocalBounds());
}
sf::FloatRect ObjAnimation::getLocalBounds() const
{
    float width = static_cast<float>( std::abs(this->g_textureRect.width) );
    float height = static_cast<float>( std::abs(this->g_textureRect.height) );

    return sf::FloatRect(0.f, 0.f, width, height);
}

void ObjAnimation::updatePositions()
{
    sf::FloatRect bounds = this->getLocalBounds();

    this->g_vertices[0].position = sf::Vector2f(0, 0);
    this->g_vertices[1].position = sf::Vector2f(0, bounds.height);
    this->g_vertices[2].position = sf::Vector2f(bounds.width, 0);
    this->g_vertices[3].position = sf::Vector2f(bounds.width, bounds.height);
}
void ObjAnimation::updateTexCoords()
{
    float left   = static_cast<float>(this->g_textureRect.left);
    float right  = left + static_cast<float>(this->g_textureRect.width);
    float top    = static_cast<float>(this->g_textureRect.top);
    float bottom = top + static_cast<float>(this->g_textureRect.height);

    this->g_vertices[0].texCoords = sf::Vector2f(left, top);
    this->g_vertices[1].texCoords = sf::Vector2f(left, bottom);
    this->g_vertices[2].texCoords = sf::Vector2f(right, top);
    this->g_vertices[3].texCoords = sf::Vector2f(right, bottom);
}

}//end fge
