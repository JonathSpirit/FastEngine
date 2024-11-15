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

#include "FastEngine/accessor/C_animation.hpp"
#include "FastEngine/manager/texture_manager.hpp"

namespace fge
{

using namespace fge::anim;

Animation::Animation(std::string_view name, Index frame) :
        BaseDataAccessor(name),
        g_frameIndex(frame)
{}
Animation::Animation(std::string_view name, std::string_view group, Index frame) :
        BaseDataAccessor(name),
        g_frameIndex(frame)
{
    this->setGroup(group);
}
Animation::Animation(char const name[], Index frame) :
        BaseDataAccessor(name),
        g_frameIndex(frame)
{}
Animation::Animation(char const name[], char const group[], Index frame) :
        BaseDataAccessor(name),
        g_frameIndex(frame)
{
    this->setGroup(group);
}
Animation::Animation(SharedDataType data, Index frame) :
        BaseDataAccessor(std::move(data)),
        g_frameIndex(frame)
{}
Animation::Animation(SharedDataType data, std::string_view group, Index frame) :
        BaseDataAccessor(std::move(data)),
        g_frameIndex(frame)
{
    this->setGroup(group);
}
Animation::Animation(SharedDataType data, char const group[], Index frame) :
        BaseDataAccessor(std::move(data)),
        g_frameIndex(frame)
{
    this->setGroup(std::string{group});
}

void Animation::clear()
{
    BaseDataAccessor::clear();

    this->g_groupIndex = 0;
    this->g_frameIndex = 0;

    this->g_loop = false;
    this->g_reverse = false;
    this->g_flipHorizontal = false;
}

fge::anim::AnimationType Animation::getType() const
{
    return this->retrieve()->_type;
}

bool Animation::setGroup(std::string_view group)
{
    auto data = this->retrieve();
    if (this->g_groupIndex < data->_groups.size())
    { //Same group
        if (data->_groups[this->g_groupIndex]._groupName == group)
        {
            this->g_frameIndex = 0;
            return true;
        }
    }

    for (std::size_t i = 0; i < data->_groups.size(); ++i)
    {
        if (data->_groups[i]._groupName == group)
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
    { //Same group
        this->g_frameIndex = 0;
        return true;
    }

    if (groupIndex < this->retrieve()->_groups.size())
    {
        this->g_groupIndex = groupIndex;
        this->g_frameIndex = 0;
        return true;
    }
    return false;
}

fge::anim::AnimationGroup const* Animation::getGroup() const
{
    auto data = this->retrieve();
    if (this->g_groupIndex < data->_groups.size())
    {
        return &data->_groups[this->g_groupIndex];
    }
    return nullptr;
}
fge::anim::AnimationGroup* Animation::getGroup()
{
    auto data = this->retrieve();
    if (this->g_groupIndex < data->_groups.size())
    {
        return &data->_groups[this->g_groupIndex];
    }
    return nullptr;
}
fge::anim::AnimationGroup const* Animation::getGroup(std::string_view group) const
{
    auto data = this->retrieve();
    for (std::size_t i = 0; i < data->_groups.size(); ++i)
    {
        if (data->_groups[i]._groupName == group)
        {
            return &data->_groups[i];
        }
    }
    return nullptr;
}
fge::anim::AnimationGroup* Animation::getGroup(std::string_view group)
{
    auto data = this->retrieve();
    for (std::size_t i = 0; i < data->_groups.size(); ++i)
    {
        if (data->_groups[i]._groupName == group)
        {
            return &data->_groups[i];
        }
    }
    return nullptr;
}
fge::anim::AnimationGroup const* Animation::getGroup(Index groupIndex) const
{
    auto data = this->retrieve();
    if (groupIndex < data->_groups.size())
    {
        return &data->_groups[groupIndex];
    }
    return nullptr;
}
fge::anim::AnimationGroup* Animation::getGroup(Index groupIndex)
{
    auto data = this->retrieve();
    if (groupIndex < data->_groups.size())
    {
        return &data->_groups[groupIndex];
    }
    return nullptr;
}

bool Animation::isGroupValid() const
{
    return this->g_groupIndex < this->retrieve()->_groups.size();
}

Animation::Index Animation::nextFrame()
{
    auto data = this->retrieve();

    if (this->g_groupIndex >= data->_groups.size())
    {
        return this->g_frameIndex;
    }

    if (this->g_reverse)
    {
        if (this->g_frameIndex == 0)
        {
            if (this->g_loop)
            {
                this->g_frameIndex = static_cast<Index>(data->_groups[this->g_groupIndex]._frames.size() - 1);
            }
        }
        else
        {
            --this->g_frameIndex;
        }
    }
    else
    {
        if (this->g_frameIndex + 1 >= static_cast<Index>(data->_groups[this->g_groupIndex]._frames.size()))
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
        return &this->retrieve()->_groups[this->g_groupIndex]._frames[this->g_frameIndex];
    }
    return nullptr;
}
fge::anim::AnimationFrame* Animation::getFrame()
{
    if (this->isFrameValid())
    {
        return &this->retrieve()->_groups[this->g_groupIndex]._frames[this->g_frameIndex];
    }
    return nullptr;
}
fge::anim::AnimationFrame const* Animation::getFrame(Index frameIndex) const
{
    auto data = this->retrieve();
    if (this->g_groupIndex < data->_groups.size())
    {
        if (frameIndex < data->_groups[this->g_groupIndex]._frames.size())
        {
            return &data->_groups[this->g_groupIndex]._frames[frameIndex];
        }
    }
    return nullptr;
}
fge::anim::AnimationFrame* Animation::getFrame(Index frameIndex)
{
    auto data = this->retrieve();
    if (this->isGroupValid())
    {
        if (frameIndex < data->_groups[this->g_groupIndex]._frames.size())
        {
            return &data->_groups[this->g_groupIndex]._frames[frameIndex];
        }
    }
    return nullptr;
}

bool Animation::isFrameValid() const
{
    auto data = this->retrieve();
    if (this->g_groupIndex < data->_groups.size())
    {
        if (this->g_frameIndex < data->_groups[this->g_groupIndex]._frames.size())
        {
            if (data->_type == fge::anim::AnimationType::ANIM_TYPE_TILESET)
            {
                return data->_groups[this->g_groupIndex]._frames[this->g_frameIndex]._texturePosition !=
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

void Animation::setHorizontalFlip(bool active)
{
    this->g_flipHorizontal = active;
}
bool Animation::isHorizontalFlipped() const
{
    return this->g_flipHorizontal;
}

std::shared_ptr<fge::TextureType> const& Animation::retrieveTexture() const
{
    auto data = this->retrieve();
    if (data->_type == fge::anim::AnimationType::ANIM_TYPE_TILESET)
    {
        if (this->isFrameValid())
        {
            return data->_tilesetTexture;
        }
    }
    else
    {
        if (this->isFrameValid())
        {
            return data->_groups[this->g_groupIndex]._frames[this->g_frameIndex]._texture;
        }
    }
    return fge::texture::gManager.getBadElement()->_ptr;
}

fge::RectInt Animation::retrieveTextureRect() const
{
    auto data = this->retrieve();
    if (data->_type == fge::anim::AnimationType::ANIM_TYPE_TILESET)
    {
        if (this->isFrameValid())
        {
            auto gridSize = static_cast<fge::Vector2i>(data->_tilesetGridSize);
            auto gridPosition = static_cast<fge::Vector2i>(
                    data->_groups[this->g_groupIndex]._frames[this->g_frameIndex]._texturePosition);
            gridPosition.x *= gridSize.x;
            gridPosition.y *= gridSize.y;

            auto rect = fge::RectInt{gridPosition, gridSize};
            if (this->g_flipHorizontal)
            {
                rect._x = gridPosition.x + gridSize.x;
                rect._width = -gridSize.x;
            }
            return rect;
        }
    }
    else
    {
        if (this->isFrameValid())
        {
            auto gridSize = static_cast<fge::Vector2i>(
                    data->_groups[this->g_groupIndex]._frames[this->g_frameIndex]._texture->getSize());

            auto rect = fge::RectInt{{0, 0}, gridSize};
            if (this->g_flipHorizontal)
            {
                rect._x = rect._width;
                rect._width = -rect._width;
            }
            return rect;
        }
    }
    return {{0, 0}, static_cast<fge::Vector2i>(fge::texture::gManager.getBadElement()->_ptr->getSize())};
}

fge::net::Packet const& operator>>(fge::net::Packet const& pck, fge::Animation& data)
{
    ///TODO: Verify extraction validity (maybe also propagate the error inside the packet ?)
    std::string name;
    Animation::Index groupIndex = 0;
    Animation::Index frameIndex = 0;
    bool loop = false;
    bool reverse = false;
    bool flipHorizontal = false;

    pck >> name >> groupIndex >> frameIndex >> loop >> reverse >> flipHorizontal;
    data = std::move(name);
    data.setGroup(groupIndex);
    data.setFrame(frameIndex);
    data.setLoop(loop);
    data.setReverse(reverse);
    data.setHorizontalFlip(flipHorizontal);
    return pck;
}
fge::net::Packet& operator<<(fge::net::Packet& pck, fge::Animation const& data)
{
    ///TODO: Use a bool "array" here
    return pck << data.getName() << data.getGroupIndex() << data.getFrameIndex() << data.isLoop() << data.isReverse()
               << data.isHorizontalFlipped();
}

void to_json(nlohmann::json& j, fge::Animation const& p)
{
    j = nlohmann::json{
            {"name", p.getName()}, {"groupIndex", p.getGroupIndex()}, {"frameIndex", p.getFrameIndex()},
            {"loop", p.isLoop()},  {"reverse", p.isReverse()},        {"flipHorizontal", p.isHorizontalFlipped()}};
}
void from_json(nlohmann::json const& j, fge::Animation& p)
{
    p = j.at("name").get<std::string>();
    p.setGroup(j.at("groupIndex").get<Animation::Index>());
    p.setFrame(j.at("frameIndex").get<Animation::Index>());
    p.setLoop(j.at("loop").get<bool>());
    p.setReverse(j.at("reverse").get<bool>());
    p.setHorizontalFlip(j.at("flipHorizontal").get<bool>());
}

} // namespace fge
