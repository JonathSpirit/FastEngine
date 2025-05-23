/*
 * Copyright 2025 Guillaume Guillet
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

#ifndef _FGE_ANIM_MANAGER_HPP_INCLUDED
#define _FGE_ANIM_MANAGER_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"

#include "FastEngine/C_vector.hpp"
#include "FastEngine/manager/C_baseManager.hpp"
#include "FastEngine/textureType.hpp"
#include <vector>

#define FGE_ANIM_DEFAULT_TICKS 100

#define FGE_ANIM_BAD FGE_MANAGER_BAD

namespace fge::anim
{

/**
 * \enum AnimationType
 * \ingroup animation
 * \brief Enum that represent different way of loading an animation
 */
enum class AnimationType
{
    ANIM_TYPE_TILESET,       ///< Tileset type, you have just one texture with multiple frame in it
    ANIM_TYPE_SEPARATE_FILES ///< Separate files, every frame is in a different file
};

/**
 * \struct AnimationFrame
 * \ingroup animation
 * \brief Structure that contains the information of a frame of an animation
 */
struct AnimationFrame
{
    std::shared_ptr<fge::TextureType> _texture; ///< The shared pointer texture of the frame
    std::filesystem::path _path;                ///< The file path of the texture
    fge::Vector2u _texturePosition; ///< The tileset grid position, only useful if the type is ANIM_TYPE_TILESET

    uint32_t _ticks; ///< The number of ticks that the frame will be displayed, by default 1 tick take 100 ms.
};

/**
 * \struct AnimationGroup
 * \ingroup animation
 * \brief Structure that contains the information of a group of frames of an animation
 */
struct AnimationGroup
{
    std::vector<AnimationFrame> _frames; ///< The vector of frames of the group
    std::string _groupName;              ///< The name of the group
};

/**
 * \struct AnimationData
 * \ingroup animation
 * \brief Structure that contains the information of an animation
 */
struct AnimationData
{
    std::vector<AnimationGroup> _groups; ///< The vector of groups of the animation

    AnimationType _type;       ///< The type of the animation
    Vector2u _tilesetGridSize; ///< The tileset grid size, only useful if the type is ANIM_TYPE_TILESET
    std::shared_ptr<TextureType> _tilesetTexture; ///< The tileset texture, only useful if the type is ANIM_TYPE_TILESET
    std::filesystem::path _tilesetPath; ///< The tileset texture path, only useful if the type is ANIM_TYPE_TILESET
};

struct DataBlock : manager::BaseDataBlock<AnimationData>
{};

/**
 * \class AnimationManager
 * \ingroup graphics
 * \brief Manage animations
 *
 * \see TextureManager
 */
class FGE_API AnimationManager : public manager::BaseManager<AnimationData, DataBlock>
{
public:
    using BaseManager::BaseManager;

    bool initialize() override;

    /**
     * \brief Load the animation with the given name from the given file path
     *
     * The specified file must be a valid json file that contains the information of the animation
     * and its groups.
     *
     * Here is an example of a valid json file:
     * \code{.json}
     * {
     *    "type": "tileset" or "separate",
     *    "gridSize": {"x": 32, "y": 32}, (not necessary if the type is separate)
     *    "tileset": "test/tileset_test.png", (not necessary if the type is separate)
     *
     *    "animationGroup1": [
     *    {
     *    "path": "path/to/the/texture1", (not necessary if the type is tileset)
     *    "ticks": 10,
     *    "position": {"x": 0, "y": 0} (not necessary if the type is separate)
     *    },
     *    {
     *    "path": "path/to/the/texture2", (not necessary if the type is tileset)
     *    "ticks": 10,
     *    "position": {"x": 1, "y": 0} (not necessary if the type is separate)
     *    }
     *    ],
     *    "animationGroup2": [
     *    ...
     *    ]
     * }
     * \endcode
     *
     * \param name The name of the animation to load
     * \param path The file path of the animation to load
     * \return \b true if the animation is loaded, \b false otherwise
     */
    bool loadFromFile(std::string_view name, std::filesystem::path const& path);
};

/**
 * \ingroup managers
 * \brief The global animation manager
 */
FGE_API extern AnimationManager gManager;

} // namespace fge::anim

#endif // _FGE_ANIM_MANAGER_HPP_INCLUDED
