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

#ifndef _FGE_C_ANIMATION_HPP_INCLUDED
#define _FGE_C_ANIMATION_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "C_rect.hpp"
#include "FastEngine/manager/C_baseManager.hpp"
#include "FastEngine/manager/anim_manager.hpp"

namespace fge
{

/**
 * \class Animation
 * \ingroup animation
 * \brief Class that represent/handle an animation
 */
class FGE_API Animation : public manager::BaseDataAccessor<
                                  manager::GlobalDataAccessorManagerInfo<anim::AnimationManager, &anim::gManager>,
                                  manager::DataAccessorOptions::BLOCKPOINTER_ONLY>
{
public:
    using BaseDataAccessor::operator=;
    using Index = uint16_t;

    Animation() = default;
    /**
     * \brief Constructor that takes the name of the animation
     *
     * This function will get the animation data from the animation manager
     * and load the first group index.
     *
     * \param name The name of the animation
     * \param frame The beginning frame of the animation
     */
    Animation(std::string_view name, Index frame = 0);
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
    Animation(std::string_view name, std::string_view group, Index frame = 0);
    Animation(char const name[], Index frame = 0);
    Animation(char const name[], char const group[], Index frame = 0);
    Animation(SharedDataType data, Index frame = 0);
    Animation(SharedDataType data, std::string_view group, Index frame = 0);
    Animation(SharedDataType data, char const group[], Index frame = 0);

    /**
     * \brief Clear the animation
     */
    void clear() override;

    /**
     * \brief Get the type of the loaded animation
     *
     * \return The type of the loaded animation
     */
    [[nodiscard]] anim::AnimationType getType() const;

    /**
     * \brief Set the group of the animation by its name
     *
     * \param group The name of the group
     * \return \b true if the group was found, \b false otherwise
     */
    bool setGroup(std::string_view group);
    /**
     * \brief Set the group of the animation by its index
     *
     * \param groupIndex The index of the group
     * \return \b true if the group was found, \b false otherwise
     */
    bool setGroup(Index groupIndex);

    /**
     * \brief Get the actual group of the animation
     *
     * \return The group data or nullptr
     */
    [[nodiscard]] anim::AnimationGroup const* getGroup() const;
    [[nodiscard]] anim::AnimationGroup* getGroup();
    /**
     * \brief Get the group of the animation by its name
     *
     * \param group The name of the group
     * \return The group data or nullptr
     */
    [[nodiscard]] anim::AnimationGroup const* getGroup(std::string_view group) const;
    [[nodiscard]] anim::AnimationGroup* getGroup(std::string_view group);
    /**
     * \brief Get the group of the animation by its index
     *
     * \param groupIndex The index of the group
     * \return The group data or nullptr
     */
    [[nodiscard]] anim::AnimationGroup const* getGroup(Index groupIndex) const;
    [[nodiscard]] anim::AnimationGroup* getGroup(Index groupIndex);

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
    Index nextFrame();
    /**
     * \brief Set the frame of the animation
     *
     * \param frame The new frame index
     */
    void setFrame(Index frame);

    /**
     * \brief Get the actual frame index of the animation
     *
     * \return The frame index
     */
    [[nodiscard]] Index getFrameIndex() const;
    /**
     * \brief Get the actual group index of the animation
     *
     * \return The group index
     */
    [[nodiscard]] Index getGroupIndex() const;

    /**
     * \brief Get the actual frame of the animation
     *
     * \return The frame data or nullptr
     */
    [[nodiscard]] fge::anim::AnimationFrame const* getFrame() const;
    [[nodiscard]] fge::anim::AnimationFrame* getFrame();
    /**
     * \brief Get the frame of the animation by its index
     *
     * \return The frame data or nullptr
     */
    [[nodiscard]] fge::anim::AnimationFrame const* getFrame(Index frameIndex) const;
    [[nodiscard]] fge::anim::AnimationFrame* getFrame(Index frameIndex);

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
     * \brief Set the horizontal flip mode of the animation
     *
     * \param active \b true to enable flip, \b false otherwise
     */
    void setHorizontalFlip(bool active);
    /**
     * \brief Check if the horizontal flip mode is active
     *
     * \return \b true if the horizontal flip mode is active, \b false otherwise
     */
    [[nodiscard]] bool isHorizontalFlipped() const;

    /**
     * \brief Retrieve the texture of the actual frame
     *
     * \return The texture or texture::GetBadTexture if something is invalid
     */
    [[nodiscard]] std::shared_ptr<fge::TextureType> const& retrieveTexture() const;

    /**
     * \brief Get the texture rectangle if the type of the animation is fge::anim::ANIM_TYPE_TILESET of the actual frame
     *
     * \return The texture rectangle or {0,0,16,16} if something is invalid
     */
    [[nodiscard]] fge::RectInt retrieveTextureRect() const;

private:
    Index g_groupIndex{0};
    Index g_frameIndex{0};

    bool g_loop{false};
    bool g_reverse{false};
    bool g_flipHorizontal{false};
};

FGE_API fge::net::Packet const& operator>>(fge::net::Packet const& pck, fge::Animation& data);
FGE_API fge::net::Packet& operator<<(fge::net::Packet& pck, fge::Animation const& data);

FGE_API void to_json(nlohmann::json& j, fge::Animation const& p);
FGE_API void from_json(nlohmann::json const& j, fge::Animation& p);

} // namespace fge

#endif // _FGE_C_ANIMATION_HPP_INCLUDED
