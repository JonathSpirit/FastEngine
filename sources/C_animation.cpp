#include "FastEngine/C_animation.hpp"
#include "FastEngine/texture_manager.hpp"

namespace fge
{

FGE_API Animation::Animation() :
    g_data( fge::anim::GetBadAnimation() ),
    g_name( FGE_ANIM_BAD ),

    g_groupIndex(0),
    g_frameIndex(0),

    g_loop(false)
{
}
FGE_API Animation::Animation(const std::string& name, std::size_t frame) :
    g_data( fge::anim::GetAnimation(name) ),
    g_name( name ),

    g_groupIndex(0),
    g_frameIndex(frame),

    g_loop(false)
{
}
FGE_API Animation::Animation(const std::string& name, const std::string& group, std::size_t frame) :
    g_data( fge::anim::GetAnimation(name) ),
    g_name( name ),

    g_groupIndex(0),
    g_frameIndex(frame),

    g_loop(false)
{
    this->setGroup(group);
}
FGE_API Animation::Animation(const char* name, std::size_t frame) :
    g_data( fge::anim::GetAnimation(std::string(name)) ),
    g_name( name ),

    g_groupIndex(0),
    g_frameIndex(frame),

    g_loop(false)
{

}
FGE_API Animation::Animation(const char* name, const char* group, std::size_t frame) :
    g_data( fge::anim::GetAnimation(std::string(name)) ),
    g_name( name ),

    g_groupIndex(0),
    g_frameIndex(frame),

    g_loop(false)
{
    this->setGroup(std::string(group));
}
FGE_API Animation::Animation(const fge::anim::AnimationDataPtr& data, std::size_t frame) :
    g_data(data),
    g_name(FGE_ANIM_BAD),

    g_groupIndex(0),
    g_frameIndex(frame),

    g_loop(false)
{

}
FGE_API Animation::Animation(const fge::anim::AnimationDataPtr& data, const std::string& group, std::size_t frame) :
    g_data(data),
    g_name(FGE_ANIM_BAD),

    g_groupIndex(0),
    g_frameIndex(frame),

    g_loop(false)
{
    this->setGroup(group);
}
FGE_API Animation::Animation(const fge::anim::AnimationDataPtr& data, const char* group, std::size_t frame) :
    g_data(data),
    g_name(FGE_ANIM_BAD),

    g_groupIndex(0),
    g_frameIndex(frame),

    g_loop(false)
{
    this->setGroup(std::string(group));
}

void FGE_API Animation::clear()
{
    this->g_data = fge::anim::GetBadAnimation();
    this->g_name = FGE_ANIM_BAD;

    this->g_groupIndex = 0;
    this->g_frameIndex = 0;

    this->g_loop = false;
}

bool FGE_API Animation::valid() const
{
    return this->g_data->_valid;
}

const std::string& FGE_API Animation::getName() const
{
    return this->g_name;
}

bool FGE_API Animation::setGroup(const std::string& groupName)
{
    for (std::size_t i=0; i<this->g_data->_groups.size(); ++i)
    {
        if ( this->g_data->_groups[i]._groupName == groupName )
        {
            this->g_groupIndex = i;
            return true;
        }
    }
    return false;
}
bool FGE_API Animation::setGroup(std::size_t groupIndex)
{
    if ( groupIndex < this->g_data->_groups.size() )
    {
        this->g_groupIndex = groupIndex;
        return true;
    }
    return false;
}

const fge::anim::AnimationGroup* FGE_API Animation::getGroup() const
{
    if ( this->g_groupIndex < this->g_data->_groups.size() )
    {
        return &this->g_data->_groups[this->g_groupIndex];
    }
    return nullptr;
}
fge::anim::AnimationGroup* FGE_API Animation::getGroup()
{
    if ( this->g_groupIndex < this->g_data->_groups.size() )
    {
        return &this->g_data->_groups[this->g_groupIndex];
    }
    return nullptr;
}
const fge::anim::AnimationGroup* FGE_API Animation::getGroup(const std::string& groupName) const
{
    for (std::size_t i=0; i<this->g_data->_groups.size(); ++i)
    {
        if ( this->g_data->_groups[i]._groupName == groupName )
        {
            return &this->g_data->_groups[i];
        }
    }
    return nullptr;
}
fge::anim::AnimationGroup* FGE_API Animation::getGroup(const std::string& groupName)
{
    for (std::size_t i=0; i<this->g_data->_groups.size(); ++i)
    {
        if ( this->g_data->_groups[i]._groupName == groupName )
        {
            return &this->g_data->_groups[i];
        }
    }
    return nullptr;
}
const fge::anim::AnimationGroup* FGE_API Animation::getGroup(std::size_t groupIndex) const
{
    if ( groupIndex < this->g_data->_groups.size() )
    {
        return &this->g_data->_groups[groupIndex];
    }
    return nullptr;
}
fge::anim::AnimationGroup* FGE_API Animation::getGroup(std::size_t groupIndex)
{
    if ( groupIndex < this->g_data->_groups.size() )
    {
        return &this->g_data->_groups[groupIndex];
    }
    return nullptr;
}

bool FGE_API Animation::isGroupValid() const
{
    if ( this->g_groupIndex < this->g_data->_groups.size() )
    {
        return true;
    }
    return false;
}

std::size_t FGE_API Animation::nextFrame()
{
    if ( this->isGroupValid() )
    {
        if ( this->g_frameIndex+1 >= this->g_data->_groups[this->g_groupIndex]._frames.size() )
        {
            if ( this->g_loop )
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
void FGE_API Animation::setFrame(std::size_t frame)
{
    this->g_frameIndex = frame;
}

std::size_t FGE_API Animation::getFrameIndex() const
{
    return this->g_frameIndex;
}
std::size_t FGE_API Animation::getGroupIndex() const
{
    return this->g_groupIndex;
}

const fge::anim::AnimationFrame* FGE_API Animation::getFrame() const
{
    if ( this->isFrameValid() )
    {
        return &this->g_data->_groups[this->g_groupIndex]._frames[this->g_frameIndex];
    }
    return nullptr;
}
fge::anim::AnimationFrame* FGE_API Animation::getFrame()
{
    if ( this->isFrameValid() )
    {
        return &this->g_data->_groups[this->g_groupIndex]._frames[this->g_frameIndex];
    }
    return nullptr;
}
const fge::anim::AnimationFrame* FGE_API Animation::getFrame(std::size_t frameIndex) const
{
    if ( this->isGroupValid() )
    {
        if ( frameIndex < this->g_data->_groups[this->g_groupIndex]._frames.size() )
        {
            return &this->g_data->_groups[this->g_groupIndex]._frames[frameIndex];
        }
    }
    return nullptr;
}
fge::anim::AnimationFrame* FGE_API Animation::getFrame(std::size_t frameIndex)
{
    if ( this->isGroupValid() )
    {
        if ( frameIndex < this->g_data->_groups[this->g_groupIndex]._frames.size() )
        {
            return &this->g_data->_groups[this->g_groupIndex]._frames[frameIndex];
        }
    }
    return nullptr;
}

bool FGE_API Animation::isFrameValid() const
{
    if ( this->isGroupValid() )
    {
        if ( this->g_frameIndex < this->g_data->_groups[this->g_groupIndex]._frames.size() )
        {
            return true;
        }
    }
    return false;
}

void FGE_API Animation::setLoop(bool active)
{
    this->g_loop = active;
}
bool FGE_API Animation::isLoop() const
{
    return this->g_loop;
}

const fge::anim::AnimationDataPtr& FGE_API Animation::getData() const
{
    return this->g_data;
}

void FGE_API Animation::operator =( const std::string& name )
{
    this->g_name = name;
    this->g_data = fge::anim::GetAnimation(name);
}
void FGE_API Animation::operator =( const char* name )
{
    this->g_name = std::string(name);
    this->g_data = fge::anim::GetAnimation(this->g_name);
}
void FGE_API Animation::operator =( const fge::anim::AnimationDataPtr& data )
{
    this->g_name = FGE_ANIM_BAD;
    this->g_data = data;
}

FGE_API Animation::operator sf::Texture*()
{
    if ( this->isFrameValid() )
    {
        return this->g_data->_groups[this->g_groupIndex]._frames[this->g_frameIndex]._texture.get();
    }
    return fge::texture::GetBadTexture()->_texture.get();
}
FGE_API Animation::operator const sf::Texture*() const
{
    if ( this->isFrameValid() )
    {
        return this->g_data->_groups[this->g_groupIndex]._frames[this->g_frameIndex]._texture.get();
    }
    return fge::texture::GetBadTexture()->_texture.get();
}

FGE_API Animation::operator sf::Texture&()
{
    if ( this->isFrameValid() )
    {
        return *this->g_data->_groups[this->g_groupIndex]._frames[this->g_frameIndex]._texture.get();
    }
    return *fge::texture::GetBadTexture()->_texture.get();
}
FGE_API Animation::operator const sf::Texture&() const
{
    if ( this->isFrameValid() )
    {
        return *this->g_data->_groups[this->g_groupIndex]._frames[this->g_frameIndex]._texture.get();
    }
    return *fge::texture::GetBadTexture()->_texture.get();
}

FGE_API Animation::operator std::string&()
{
    return this->g_name;
}
FGE_API Animation::operator const std::string&() const
{
    return this->g_name;
}

}//end fge
