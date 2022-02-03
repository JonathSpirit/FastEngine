#ifndef _FGE_C_ANIMATION_HPP_INCLUDED
#define _FGE_C_ANIMATION_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <FastEngine/anim_manager.hpp>

namespace fge
{

class FGE_API Animation
{
public:
    Animation();
    Animation(const std::string& name, std::size_t frame=0);
    Animation(const std::string& name, const std::string& group, std::size_t frame=0);
    Animation(const char* name, std::size_t frame=0);
    Animation(const char* name, const char* group, std::size_t frame=0);
    Animation(const fge::anim::AnimationDataPtr& data, std::size_t frame=0);
    Animation(const fge::anim::AnimationDataPtr& data, const std::string& group, std::size_t frame=0);
    Animation(const fge::anim::AnimationDataPtr& data, const char* group, std::size_t frame=0);

    void clear();

    bool valid() const;

    const std::string& getName() const;

    bool setGroup(const std::string& groupName);
    bool setGroup(std::size_t groupIndex);

    const fge::anim::AnimationGroup* getGroup() const;
    fge::anim::AnimationGroup* getGroup();
    const fge::anim::AnimationGroup* getGroup(const std::string& groupName) const;
    fge::anim::AnimationGroup* getGroup(const std::string& groupName);
    const fge::anim::AnimationGroup* getGroup(std::size_t groupIndex) const;
    fge::anim::AnimationGroup* getGroup(std::size_t groupIndex);

    bool isGroupValid() const;

    std::size_t nextFrame();
    void setFrame(std::size_t frame);

    std::size_t getFrameIndex() const;
    std::size_t getGroupIndex() const;

    const fge::anim::AnimationFrame* getFrame() const;
    fge::anim::AnimationFrame* getFrame();
    const fge::anim::AnimationFrame* getFrame(std::size_t frameIndex) const;
    fge::anim::AnimationFrame* getFrame(std::size_t frameIndex);

    bool isFrameValid() const;

    void setLoop(bool active);
    bool isLoop() const;

    const fge::anim::AnimationDataPtr& getData() const;

    void operator =( const std::string& name );
    void operator =( const char* name );
    void operator =( const fge::anim::AnimationDataPtr& data );

    explicit operator sf::Texture*();
    explicit operator const sf::Texture*() const;

    operator sf::Texture&();
    operator const sf::Texture&() const;

    operator std::string&();
    operator const std::string&() const;

private:
    fge::anim::AnimationDataPtr g_data;
    std::string g_name;

    std::size_t g_groupIndex;
    std::size_t g_frameIndex;

    bool g_loop;
};

}//end fge

#endif // _FGE_C_ANIMATION_HPP_INCLUDED
