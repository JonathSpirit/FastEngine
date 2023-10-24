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

#include "FastEngine/object/C_objAnim.hpp"

namespace fge
{

ObjAnimation::ObjAnimation() :
        g_vertices(fge::vulkan::GetActiveContext()),
        g_tickDuration(std::chrono::milliseconds{FGE_OBJANIM_DEFAULT_TICKDURATION_MS}),
        g_nextFrameTime(0),

        g_paused(false)
{
    this->g_vertices.create(4, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, fge::vulkan::BufferTypes::LOCAL);
    this->setTextureRect(this->g_animation);
}
ObjAnimation::ObjAnimation(fge::Animation const& animation, fge::Vector2f const& position) :
        g_vertices(fge::vulkan::GetActiveContext()),
        g_animation(animation),
        g_tickDuration(std::chrono::milliseconds{FGE_OBJANIM_DEFAULT_TICKDURATION_MS}),
        g_nextFrameTime(0),

        g_paused(false)
{
    this->g_vertices.create(4, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, fge::vulkan::BufferTypes::LOCAL);
    this->setPosition(position);
    this->setTextureRect(this->g_animation);
}

void ObjAnimation::setAnimation(fge::Animation const& animation)
{
    this->g_animation = animation;
    this->setTextureRect(this->g_animation);
}
void ObjAnimation::setTextureRect(fge::RectInt const& rectangle)
{
    if (rectangle != this->g_textureRect)
    {
        this->g_textureRect = rectangle;
        this->updatePositions();
        this->updateTexCoords();
    }
}

void ObjAnimation::setColor(fge::Color const& color)
{
    this->g_vertices[0]._color = color;
    this->g_vertices[1]._color = color;
    this->g_vertices[2]._color = color;
    this->g_vertices[3]._color = color;
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

void ObjAnimation::setTickDuration(std::chrono::milliseconds const& tms)
{
    this->g_tickDuration = tms;
}
std::chrono::microseconds const& ObjAnimation::getTickDuration() const
{
    return this->g_tickDuration;
}

fge::Animation const& ObjAnimation::getAnimation() const
{
    return this->g_animation;
}
fge::Animation& ObjAnimation::getAnimation()
{
    return this->g_animation;
}
fge::RectInt const& ObjAnimation::getTextureRect() const
{
    return this->g_textureRect;
}

fge::Color ObjAnimation::getColor() const
{
    return fge::Color(this->g_vertices[0]._color);
}

FGE_OBJ_UPDATE_BODY(ObjAnimation)
{
    if (!this->g_paused)
    {
        fge::anim::AnimationFrame* frame = this->g_animation.getFrame();
        if (frame != nullptr)
        {
            this->g_nextFrameTime += deltaTime;
            if (this->g_nextFrameTime >= std::chrono::microseconds{this->g_tickDuration * frame->_ticks})
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

#ifndef FGE_DEF_SERVER
FGE_OBJ_DRAW_BODY(ObjAnimation)
{
    auto copyStates = states.copy(this->_transform.start(*this, states._resTransform.get()));

    copyStates._vertexBuffer = &this->g_vertices;
    copyStates._resTextures.set(this->g_animation.retrieveTexture().get(), 1);
    target.draw(copyStates);
}
#endif

void ObjAnimation::save(nlohmann::json& jsonObject, fge::Scene* scene)
{
    fge::Object::save(jsonObject, scene);

    jsonObject["color"] = fge::Color(this->g_vertices[0]._color).toInteger();
    jsonObject["animation"] = this->g_animation;
    jsonObject["animationGroup"] = this->g_animation.getGroupIndex();
    jsonObject["animationFrame"] = this->g_animation.getFrameIndex();
    jsonObject["animationLoop"] = this->g_animation.isLoop();
    jsonObject["animationReverse"] = this->g_animation.isReverse();
    jsonObject["tickDuration"] = static_cast<uint16_t>(this->g_tickDuration.count());
}
void ObjAnimation::load(nlohmann::json& jsonObject, fge::Scene* scene)
{
    fge::Object::load(jsonObject, scene);

    this->setColor(fge::Color(jsonObject.value<uint32_t>("color", 0)));
    this->g_animation = jsonObject.value<std::string>("animation", FGE_ANIM_BAD);
    this->g_animation.setGroup(jsonObject.value<std::size_t>("animationGroup", 0));
    this->g_animation.setFrame(jsonObject.value<std::size_t>("animationFrame", 0));
    this->g_animation.setLoop(jsonObject.value<bool>("animationLoop", false));
    this->g_animation.setReverse(jsonObject.value<bool>("animationReverse", false));
    this->g_tickDuration =
            std::chrono::milliseconds(jsonObject.value<uint16_t>("tickDuration", FGE_OBJANIM_DEFAULT_TICKDURATION_MS));

    this->setTextureRect(this->g_animation);
}
void ObjAnimation::pack(fge::net::Packet& pck)
{
    fge::Object::pack(pck);

    pck << this->g_vertices[0]._color << this->g_animation;
    pck << static_cast<uint32_t>(this->g_animation.getGroupIndex())
        << static_cast<uint32_t>(this->g_animation.getFrameIndex());
    pck << this->g_animation.isLoop() << this->g_animation.isReverse();
    pck << static_cast<uint16_t>(this->g_tickDuration.count());
}
void ObjAnimation::unpack(fge::net::Packet const& pck)
{
    fge::Object::unpack(pck);

    fge::Color color;
    pck >> color >> this->g_animation;
    this->setColor(color);
    uint32_t group = 0, frame = 0;
    bool loop = false, reverse = false;
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

char const* ObjAnimation::getClassName() const
{
    return FGE_OBJANIM_CLASSNAME;
}
char const* ObjAnimation::getReadableClassName() const
{
    return "animation";
}

fge::RectFloat ObjAnimation::getGlobalBounds() const
{
    return this->getTransform() * this->getLocalBounds();
}
fge::RectFloat ObjAnimation::getLocalBounds() const
{
    auto const width = static_cast<float>(std::abs(this->g_textureRect._width));
    auto const height = static_cast<float>(std::abs(this->g_textureRect._height));

    return {{0.f, 0.f}, {width, height}};
}

void ObjAnimation::updatePositions()
{
    fge::RectFloat const bounds = this->getLocalBounds();

    this->g_vertices[0]._position = fge::Vector2f(0, 0);
    this->g_vertices[1]._position = fge::Vector2f(0, bounds._height);
    this->g_vertices[2]._position = fge::Vector2f(bounds._width, 0);
    this->g_vertices[3]._position = fge::Vector2f(bounds._width, bounds._height);
}
void ObjAnimation::updateTexCoords()
{
    auto const rect = this->g_animation.retrieveTexture()->normalizeTextureRect(this->g_textureRect);

    this->g_vertices[0]._texCoords = fge::Vector2f(rect._x, rect._y);
    this->g_vertices[1]._texCoords = fge::Vector2f(rect._x, rect._y + rect._height);
    this->g_vertices[2]._texCoords = fge::Vector2f(rect._x + rect._width, rect._y);
    this->g_vertices[3]._texCoords = fge::Vector2f(rect._x + rect._width, rect._y + rect._height);
}

} // namespace fge
