#ifndef _FGE_C_ANIMATION_HPP_INCLUDED
#define _FGE_C_ANIMATION_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <FastEngine/anim_manager.hpp>

namespace fge
{

/**
 * \class Animation
 * \ingroup animation
 * \brief Class that represent/handle an animation
 */
class FGE_API Animation
{
public:
    Animation();
    /**
     * \brief Constructor that takes the name of the animation
     *
     * This function will get the animation data from the animation manager
     * and load the first group index.
     *
     * \param name The name of the animation
     * \param frame The beginning frame of the animation
     */
    Animation(const std::string& name, std::size_t frame=0);
    /**
     * \brief Constructor that takes the name of the animation and the group name
     *
     * This function will get the animation data from the animation manager
     * and load the wanted group by its name.
     *
     * \param name The name of the animation
     * \param group The name of the group
     * \param frame The beginning frame of the animation
     */
    Animation(const std::string& name, const std::string& group, std::size_t frame=0);
    Animation(const char* name, std::size_t frame=0);
    Animation(const char* name, const char* group, std::size_t frame=0);
    Animation(const fge::anim::AnimationDataPtr& data, std::size_t frame=0);
    Animation(const fge::anim::AnimationDataPtr& data, const std::string& group, std::size_t frame=0);
    Animation(const fge::anim::AnimationDataPtr& data, const char* group, std::size_t frame=0);

    /**
     * \brief Clear the animation
     */
    void clear();

    /**
     * \brief Check if the animation data is valid
     *
     * \return The validity of the animation data
     */
    [[nodiscard]] bool valid() const;

    /**
     * \brief Get the name of the loaded animation
     *
     * \return The name of the loaded animation
     */
    [[nodiscard]] const std::string& getName() const;

    /**
     * \brief Get the type of the loaded animation
     *
     * \return The type of the loaded animation
     */
    [[nodiscard]] fge::anim::AnimationType getType() const;

    /**
     * \brief Set the group of the animation by its name
     *
     * \param groupName The name of the group
     * \return \b true if the group was found, \b false otherwise
     */
    bool setGroup(const std::string& groupName);
    /**
     * \brief Set the group of the animation by its index
     *
     * \param groupIndex The index of the group
     * \return \b true if the group was found, \b false otherwise
     */
    bool setGroup(std::size_t groupIndex);

    /**
     * \brief Get the actual group of the animation
     *
     * \return The group data or nullptr
     */
    [[nodiscard]] const fge::anim::AnimationGroup* getGroup() const;
    [[nodiscard]] fge::anim::AnimationGroup* getGroup();
    /**
     * \brief Get the group of the animation by its name
     *
     * \param groupName The name of the group
     * \return The group data or nullptr
     */
    [[nodiscard]] const fge::anim::AnimationGroup* getGroup(const std::string& groupName) const;
    [[nodiscard]] fge::anim::AnimationGroup* getGroup(const std::string& groupName);
    /**
     * \brief Get the group of the animation by its index
     *
     * \param groupIndex The index of the group
     * \return The group data or nullptr
     */
    [[nodiscard]] const fge::anim::AnimationGroup* getGroup(std::size_t groupIndex) const;
    [[nodiscard]] fge::anim::AnimationGroup* getGroup(std::size_t groupIndex);

    /**
     * \brief Check if the actual group is valid
     *
     * \return The validity of the actual group
     */
    [[nodiscard]] bool isGroupValid() const;

    /**
     * \brief Go to the next frame of the animation
     *
     * \return The new frame index
     */
    std::size_t nextFrame();
    /**
     * \brief Set the frame of the animation
     *
     * \param frame The new frame index
     */
    void setFrame(std::size_t frame);

    /**
     * \brief Get the actual frame index of the animation
     *
     * \return The frame index
     */
    [[nodiscard]] std::size_t getFrameIndex() const;
    /**
     * \brief Get the actual group index of the animation
     *
     * \return The group index
     */
    [[nodiscard]] std::size_t getGroupIndex() const;

    /**
     * \brief Get the actual frame of the animation
     *
     * \return The frame data or nullptr
     */
    [[nodiscard]] const fge::anim::AnimationFrame* getFrame() const;
    [[nodiscard]] fge::anim::AnimationFrame* getFrame();
    /**
     * \brief Get the frame of the animation by its index
     *
     * \return The frame data or nullptr
     */
    [[nodiscard]] const fge::anim::AnimationFrame* getFrame(std::size_t frameIndex) const;
    [[nodiscard]] fge::anim::AnimationFrame* getFrame(std::size_t frameIndex);

    /**
     * \brief Check if the actual frame is valid
     *
     * \return The validity of the actual frame
     */
    [[nodiscard]] bool isFrameValid() const;

    /**
     * \brief Set the loop mode of the animation
     *
     * \param active \b true to enable the loop mode, \b false otherwise
     */
    void setLoop(bool active);
    /**
     * \brief Check if the loop mode is active
     *
     * \return \b true if the loop mode is active, \b false otherwise
     */
    [[nodiscard]] bool isLoop() const;

    /**
     * \brief Set the reverse mode of the animation
     *
     * \param active \b true to enable the reverse mode, \b false otherwise
     */
    void setReverse(bool active);
    /**
     * \brief Check if the reverse mode is active
     *
     * \return \b true if the reverse mode is active, \b false otherwise
     */
    [[nodiscard]] bool isReverse() const;

    /**
     * \brief Get the load animation data
     *
     * \return The animation data
     */
    [[nodiscard]] const fge::anim::AnimationDataPtr& getData() const;

    fge::Animation& operator =( const std::string& name );
    fge::Animation& operator =( const char* name );
    fge::Animation& operator =( const fge::anim::AnimationDataPtr& data );

    /**
     * \brief Get the texture of the actual frame
     *
     * \return The SFML texture or texture::GetBadTexture if something is invalid
     */
    explicit operator sf::Texture*();
    explicit operator const sf::Texture*() const;

    operator sf::Texture&();
    operator const sf::Texture&() const;

    operator std::string&();
    operator const std::string&() const;

    /**
     * \brief Get the texture rectangle if the type of the animation is fge::anim::ANIM_TYPE_TILESET of the actual frame
     *
     * \return The SFML texture rectangle or {0,0,16,16} if something is invalid
     */
    operator sf::IntRect() const;

private:
    fge::anim::AnimationDataPtr g_data;
    std::string g_name;

    std::size_t g_groupIndex;
    std::size_t g_frameIndex;

    bool g_loop;
    bool g_reverse;
};

}//end fge

#endif // _FGE_C_ANIMATION_HPP_INCLUDED
