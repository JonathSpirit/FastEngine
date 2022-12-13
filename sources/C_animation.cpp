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

#include "FastEngine/C_animation.hpp"
#include "FastEngine/manager/texture_manager.hpp"

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
Animation::Animation(const std::string& name, std::size_t frame) :
        g_data(fge::anim::GetAnimation(name)),
        g_name(name),

        g_groupIndex(0),
        g_frameIndex(frame),

        g_loop(false),
        g_reverse(false)
{}
Animation::Animation(const std::string& name, const std::string& group, std::size_t frame) :
        g_data(fge::anim::GetAnimation(name)),
        g_name(name),

        g_groupIndex(0),
        g_frameIndex(frame),

        g_loop(false),
        g_reverse(false)
{
    this->setGroup(group);
}
Animation::Animation(const char* name, std::size_t frame) :
        g_data(fge::anim::GetAnimation(std::string(name))),
        g_name(name),

        g_groupIndex(0),
        g_frameIndex(frame),

        g_loop(false),
        g_reverse(false)
{}
Animation::Animation(const char* name, const char* group, std::size_t frame) :
        g_data(fge::anim::GetAnimation(std::string(name))),
        g_name(name),

        g_groupIndex(0),
        g_frameIndex(frame),

        g_loop(false),
        g_reverse(false)
{
    this->setGroup(std::string(group));
}
Animation::Animation(const fge::anim::AnimationDataPtr& data, std::size_t frame) :
        g_data(data),
        g_name(FGE_ANIM_BAD),

        g_groupIndex(0),
        g_frameIndex(frame),

        g_loop(false),
        g_reverse(false)
{}
Animation::Animation(const fge::anim::AnimationDataPtr& data, const std::string& group, std::size_t frame) :
        g_data(data),
        g_name(FGE_ANIM_BAD),

        g_groupIndex(0),
        g_frameIndex(frame),

        g_loop(false),
        g_reverse(false)
{
    this->setGroup(group);
}
Animation::Animation(const fge::anim::AnimationDataPtr& data, const char* group, std::size_t frame) :
        g_data(data),
        g_name(FGE_ANIM_BAD),

        g_groupIndex(0),
        g_frameIndex(frame),

        g_loop(false),
        g_reverse(false)
{
    this->setGroup(std::string(group));
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

const std::string& Animation::getName() const
{
    return this->g_name;
}

fge::anim::AnimationType Animation::getType() const
{
    return this->g_data->_type;
}

bool Animation::setGroup(const std::string& groupName)
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
bool Animation::setGroup(std::size_t groupIndex)
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

const fge::anim::AnimationGroup* Animation::getGroup() const
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
const fge::anim::AnimationGroup* Animation::getGroup(const std::string& groupName) const
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
fge::anim::AnimationGroup* Animation::getGroup(const std::string& groupName)
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
const fge::anim::AnimationGroup* Animation::getGroup(std::size_t groupIndex) const
{
    if (groupIndex < this->g_data->_groups.size())
    {
        return &this->g_data->_groups[groupIndex];
    }
    return nullptr;
}
fge::anim::AnimationGroup* Animation::getGroup(std::size_t groupIndex)
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

std::size_t Animation::nextFrame()
{
    if (this->isGroupValid())
    {
        if (this->g_reverse)
        {
            if (this->g_frameIndex == 0)
            {
                if (this->g_loop)
                {
                    this->g_frameIndex = this->g_data->_groups[this->g_groupIndex]._frames.size() - 1;
                }
            }
            else
            {
                --this->g_frameIndex;
            }
        }
        else
        {
            if (this->g_frameIndex + 1 >= this->g_data->_groups[this->g_groupIndex]._frames.size())
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
void Animation::setFrame(std::size_t frame)
{
    this->g_frameIndex = frame;
}

std::size_t Animation::getFrameIndex() const
{
    return this->g_frameIndex;
}
std::size_t Animation::getGroupIndex() const
{
    return this->g_groupIndex;
}

const fge::anim::AnimationFrame* Animation::getFrame() const
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
const fge::anim::AnimationFrame* Animation::getFrame(std::size_t frameIndex) const
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
fge::anim::AnimationFrame* Animation::getFrame(std::size_t frameIndex)
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

const fge::anim::AnimationDataPtr& Animation::getData() const
{
    return this->g_data;
}

fge::Animation& Animation::operator=(const std::string& name)
{
    this->g_name = name;
    this->g_data = fge::anim::GetAnimation(name);
    return *this;
}
fge::Animation& Animation::operator=(const char* name)
{
    this->g_name = std::string(name);
    this->g_data = fge::anim::GetAnimation(this->g_name);
    return *this;
}
fge::Animation& Animation::operator=(const fge::anim::AnimationDataPtr& data)
{
    this->g_name = FGE_ANIM_BAD;
    this->g_data = data;
    return *this;
}

Animation::operator fge::TextureType*()
{
    if (this->g_data->_type == fge::anim::AnimationType::ANIM_TYPE_TILESET)
    {
        if (this->g_data->_valid)
        {
            return this->g_data->_tilesetTexture.get();
        }
    }
    else
    {
        if (this->isFrameValid())
        {
            return this->g_data->_groups[this->g_groupIndex]._frames[this->g_frameIndex]._texture.get();
        }
    }
    return fge::texture::GetBadTexture()->_texture.get();
}
Animation::operator const fge::TextureType*() const
{
    if (this->g_data->_type == fge::anim::AnimationType::ANIM_TYPE_TILESET)
    {
        if (this->g_data->_valid)
        {
            return this->g_data->_tilesetTexture.get();
        }
    }
    else
    {
        if (this->isFrameValid())
        {
            return this->g_data->_groups[this->g_groupIndex]._frames[this->g_frameIndex]._texture.get();
        }
    }
    return fge::texture::GetBadTexture()->_texture.get();
}

Animation::operator fge::TextureType&()
{
    if (this->g_data->_type == fge::anim::AnimationType::ANIM_TYPE_TILESET)
    {
        if (this->g_data->_valid)
        {
            return *this->g_data->_tilesetTexture;
        }
    }
    else
    {
        if (this->isFrameValid())
        {
            return *this->g_data->_groups[this->g_groupIndex]._frames[this->g_frameIndex]._texture;
        }
    }
    return *fge::texture::GetBadTexture()->_texture;
}
Animation::operator const fge::TextureType&() const
{
    if (this->g_data->_type == fge::anim::AnimationType::ANIM_TYPE_TILESET)
    {
        if (this->g_data->_valid)
        {
            return *this->g_data->_tilesetTexture;
        }
    }
    else
    {
        if (this->isFrameValid())
        {
            return *this->g_data->_groups[this->g_groupIndex]._frames[this->g_frameIndex]._texture;
        }
    }
    return *fge::texture::GetBadTexture()->_texture;
}

Animation::operator std::string&()
{
    return this->g_name;
}
Animation::operator const std::string&() const
{
    return this->g_name;
}

Animation::operator sf::IntRect() const
{
    if (this->g_data->_type == fge::anim::AnimationType::ANIM_TYPE_TILESET)
    {
        if (this->isFrameValid())
        {
            auto gridSize = static_cast<sf::Vector2i>(this->g_data->_tilesetGridSize);
            auto gridPosition = static_cast<sf::Vector2i>(
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
            auto gridSize = static_cast<sf::Vector2i>(
                    this->g_data->_groups[this->g_groupIndex]._frames[this->g_frameIndex]._texture->getSize());
            return {{0, 0}, gridSize};
        }
    }
    return {{0, 0}, static_cast<sf::Vector2i>(fge::texture::GetBadTexture()->_texture->getSize())};
}

} // namespace fge
