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

#include "FastEngine/C_animation.hpp"
#include "FastEngine/manager/texture_manager.hpp"
#include "FastEngine/network/C_packet.hpp"

namespace fge
{

Animation::Animation() :
        g_data(fge::anim::GetBadAnimation()),
        g_name(FGE_ANIM_BAD),

        g_groupIndex(0),
        g_frameIndex(0),

        g_loop(false),
        g_reverse(false)
{}
Animation::Animation(std::string name, std::size_t frame) :
        g_data(fge::anim::GetAnimation(name)),
        g_name(std::move(name)),

        g_groupIndex(0),
        g_frameIndex(frame),

        g_loop(false),
        g_reverse(false)
{}
Animation::Animation(std::string name, std::string const& group, Index frame) :
        g_data(fge::anim::GetAnimation(name)),
        g_name(std::move(name)),

        g_groupIndex(0),
        g_frameIndex(frame),

        g_loop(false),
        g_reverse(false)
{
    this->setGroup(group);
}
Animation::Animation(char const* name, Index frame) :
        g_data(fge::anim::GetAnimation(std::string(name))),
        g_name(name),

        g_groupIndex(0),
        g_frameIndex(frame),

        g_loop(false),
        g_reverse(false)
{}
Animation::Animation(char const* name, char const* group, Index frame) :
        g_data(fge::anim::GetAnimation(std::string(name))),
        g_name(name),

        g_groupIndex(0),
        g_frameIndex(frame),

        g_loop(false),
        g_reverse(false)
{
    this->setGroup(std::string{group});
}
Animation::Animation(fge::anim::AnimationDataPtr data, Index frame) :
        g_data(std::move(data)),
        g_name(FGE_ANIM_BAD),

        g_groupIndex(0),
        g_frameIndex(frame),

        g_loop(false),
        g_reverse(false)
{}
Animation::Animation(fge::anim::AnimationDataPtr data, std::string const& group, Index frame) :
        g_data(std::move(data)),
        g_name(FGE_ANIM_BAD),

        g_groupIndex(0),
        g_frameIndex(frame),

        g_loop(false),
        g_reverse(false)
{
    this->setGroup(group);
}
Animation::Animation(fge::anim::AnimationDataPtr data, char const* group, Index frame) :
        g_data(std::move(data)),
        g_name(FGE_ANIM_BAD),

        g_groupIndex(0),
        g_frameIndex(frame),

        g_loop(false),
        g_reverse(false)
{
    this->setGroup(std::string{group});
}

void Animation::clear()
{
    this->g_data = fge::anim::GetBadAnimation();
    this->g_name = FGE_ANIM_BAD;

    this->g_groupIndex = 0;
    this->g_frameIndex = 0;

    this->g_loop = false;
    this->g_reverse = false;
}

bool Animation::valid() const
{
    return this->g_data->_valid;
}

std::string const& Animation::getName() const
{
    return this->g_name;
}

fge::anim::AnimationType Animation::getType() const
{
    return this->g_data->_type;
}

bool Animation::setGroup(std::string const& groupName)
{
    if (this->isGroupValid())
    { //If same group, we do nothing
        if (this->g_data->_groups[this->g_groupIndex]._groupName == groupName)
        {
            return false;
        }
    }

    for (std::size_t i = 0; i < this->g_data->_groups.size(); ++i)
    {
        if (this->g_data->_groups[i]._groupName == groupName)
        {
            this->g_groupIndex = i;
            this->g_frameIndex = 0;
            return true;
        }
    }
    return false;
}
bool Animation::setGroup(Index groupIndex)
{
    if (this->g_groupIndex == groupIndex)
    { //If same group, we do nothing
        return false;
    }

    if (groupIndex < this->g_data->_groups.size())
    {
        this->g_groupIndex = groupIndex;
        this->g_frameIndex = 0;
        return true;
    }
    return false;
}

fge::anim::AnimationGroup const* Animation::getGroup() const
{
    if (this->g_groupIndex < this->g_data->_groups.size())
    {
        return &this->g_data->_groups[this->g_groupIndex];
    }
    return nullptr;
}
fge::anim::AnimationGroup* Animation::getGroup()
{
    if (this->g_groupIndex < this->g_data->_groups.size())
    {
        return &this->g_data->_groups[this->g_groupIndex];
    }
    return nullptr;
}
fge::anim::AnimationGroup const* Animation::getGroup(std::string const& groupName) const
{
    for (std::size_t i = 0; i < this->g_data->_groups.size(); ++i)
    {
        if (this->g_data->_groups[i]._groupName == groupName)
        {
            return &this->g_data->_groups[i];
        }
    }
    return nullptr;
}
fge::anim::AnimationGroup* Animation::getGroup(std::string const& groupName)
{
    for (std::size_t i = 0; i < this->g_data->_groups.size(); ++i)
    {
        if (this->g_data->_groups[i]._groupName == groupName)
        {
            return &this->g_data->_groups[i];
        }
    }
    return nullptr;
}
fge::anim::AnimationGroup const* Animation::getGroup(Index groupIndex) const
{
    if (groupIndex < this->g_data->_groups.size())
    {
        return &this->g_data->_groups[groupIndex];
    }
    return nullptr;
}
fge::anim::AnimationGroup* Animation::getGroup(Index groupIndex)
{
    if (groupIndex < this->g_data->_groups.size())
    {
        return &this->g_data->_groups[groupIndex];
    }
    return nullptr;
}

bool Animation::isGroupValid() const
{
    return this->g_groupIndex < this->g_data->_groups.size();
}

Animation::Index Animation::nextFrame()
{
    if (this->isGroupValid())
    {
        if (this->g_reverse)
        {
            if (this->g_frameIndex == 0)
            {
                if (this->g_loop)
                {
                    this->g_frameIndex =
                            static_cast<Index>(this->g_data->_groups[this->g_groupIndex]._frames.size() - 1);
                }
            }
            else
            {
                --this->g_frameIndex;
            }
        }
        else
        {
            if (this->g_frameIndex + 1 >= static_cast<Index>(this->g_data->_groups[this->g_groupIndex]._frames.size()))
            {
                if (this->g_loop)
                {
                    this->g_frameIndex = 0;
                }
            }
            else
            {
                ++this->g_frameIndex;
            }
        }
    }
    return this->g_frameIndex;
}
void Animation::setFrame(Index frame)
{
    this->g_frameIndex = frame;
}

Animation::Index Animation::getFrameIndex() const
{
    return this->g_frameIndex;
}
Animation::Index Animation::getGroupIndex() const
{
    return this->g_groupIndex;
}

fge::anim::AnimationFrame const* Animation::getFrame() const
{
    if (this->isFrameValid())
    {
        return &this->g_data->_groups[this->g_groupIndex]._frames[this->g_frameIndex];
    }
    return nullptr;
}
fge::anim::AnimationFrame* Animation::getFrame()
{
    if (this->isFrameValid())
    {
        return &this->g_data->_groups[this->g_groupIndex]._frames[this->g_frameIndex];
    }
    return nullptr;
}
fge::anim::AnimationFrame const* Animation::getFrame(Index frameIndex) const
{
    if (this->isGroupValid())
    {
        if (frameIndex < this->g_data->_groups[this->g_groupIndex]._frames.size())
        {
            return &this->g_data->_groups[this->g_groupIndex]._frames[frameIndex];
        }
    }
    return nullptr;
}
fge::anim::AnimationFrame* Animation::getFrame(Index frameIndex)
{
    if (this->isGroupValid())
    {
        if (frameIndex < this->g_data->_groups[this->g_groupIndex]._frames.size())
        {
            return &this->g_data->_groups[this->g_groupIndex]._frames[frameIndex];
        }
    }
    return nullptr;
}

bool Animation::isFrameValid() const
{
    if (this->isGroupValid())
    {
        if (this->g_frameIndex < this->g_data->_groups[this->g_groupIndex]._frames.size())
        {
            if (this->g_data->_type == fge::anim::AnimationType::ANIM_TYPE_TILESET)
            {
                return this->g_data->_groups[this->g_groupIndex]._frames[this->g_frameIndex]._texturePosition !=
                       FGE_NUMERIC_LIMITS_VECTOR_MAX(fge::Vector2u);
            }
            return true;
        }
    }
    return false;
}

void Animation::setLoop(bool active)
{
    this->g_loop = active;
}
bool Animation::isLoop() const
{
    return this->g_loop;
}

void Animation::setReverse(bool active)
{
    this->g_reverse = active;
}
bool Animation::isReverse() const
{
    return this->g_reverse;
}

fge::anim::AnimationDataPtr const& Animation::getData() const
{
    return this->g_data;
}

fge::Animation& Animation::operator=(std::string name)
{
    this->g_name = std::move(name);
    this->g_data = fge::anim::GetAnimation(name);
    return *this;
}
fge::Animation& Animation::operator=(char const* name)
{
    this->g_name = std::string{name};
    this->g_data = fge::anim::GetAnimation(this->g_name);
    return *this;
}
fge::Animation& Animation::operator=(fge::anim::AnimationDataPtr data)
{
    this->g_name = FGE_ANIM_BAD;
    this->g_data = std::move(data);
    return *this;
}

std::shared_ptr<fge::TextureType> const& Animation::retrieveTexture() const
{
    if (this->g_data->_type == fge::anim::AnimationType::ANIM_TYPE_TILESET)
    {
        if (this->isFrameValid())
        {
            return this->g_data->_tilesetTexture;
        }
    }
    else
    {
        if (this->isFrameValid())
        {
            return this->g_data->_groups[this->g_groupIndex]._frames[this->g_frameIndex]._texture;
        }
    }
    return fge::texture::GetBadTexture()->_texture;
}

Animation::operator fge::RectInt() const
{
    if (this->g_data->_type == fge::anim::AnimationType::ANIM_TYPE_TILESET)
    {
        if (this->isFrameValid())
        {
            auto gridSize = static_cast<fge::Vector2i>(this->g_data->_tilesetGridSize);
            auto gridPosition = static_cast<fge::Vector2i>(
                    this->g_data->_groups[this->g_groupIndex]._frames[this->g_frameIndex]._texturePosition);
            gridPosition.x *= gridSize.x;
            gridPosition.y *= gridSize.y;
            return {gridPosition, gridSize};
        }
    }
    else
    {
        if (this->isFrameValid())
        {
            auto gridSize = static_cast<fge::Vector2i>(
                    this->g_data->_groups[this->g_groupIndex]._frames[this->g_frameIndex]._texture->getSize());
            return {{0, 0}, gridSize};
        }
    }
    return {{0, 0}, static_cast<fge::Vector2i>(fge::texture::GetBadTexture()->_texture->getSize())};
}

fge::net::Packet const& operator>>(fge::net::Packet const& pck, fge::Animation& data)
{
    ///TODO: Verify extraction validity (maybe also propagate the error inside the packet ?)
    std::string name;
    Animation::Index groupIndex = 0;
    Animation::Index frameIndex = 0;
    bool loop = false;
    bool reverse = false;

    pck >> name >> groupIndex >> frameIndex >> loop >> reverse;
    data = std::move(name);
    data.setGroup(groupIndex);
    data.setFrame(frameIndex);
    data.setLoop(loop);
    data.setReverse(reverse);
    return pck;
}
fge::net::Packet& operator<<(fge::net::Packet& pck, fge::Animation const& data)
{
    ///TODO: Use a bool "array" here
    return pck << data.getName() << data.getGroupIndex() << data.getFrameIndex() << data.isLoop() << data.isReverse();
}

void to_json(nlohmann::json& j, fge::Animation const& p)
{
    j = nlohmann::json{{"name", p.getName()},
                       {"groupIndex", p.getGroupIndex()},
                       {"frameIndex", p.getFrameIndex()},
                       {"loop", p.isLoop()},
                       {"reverse", p.isReverse()}};
}
void from_json(nlohmann::json const& j, fge::Animation& p)
{
    p = j.at("name").get<std::string>();
    p.setGroup(j.at("groupIndex").get<Animation::Index>());
    p.setFrame(j.at("frameIndex").get<Animation::Index>());
    p.setLoop(j.at("loop").get<bool>());
    p.setReverse(j.at("reverse").get<bool>());
}

} // namespace fge
