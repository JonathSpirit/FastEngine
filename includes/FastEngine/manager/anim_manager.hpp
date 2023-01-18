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

#ifndef _FGE_ANIM_MANAGER_HPP_INCLUDED
#define _FGE_ANIM_MANAGER_HPP_INCLUDED

#include "FastEngine/fastengine_extern.hpp"

#include "FastEngine/C_vector.hpp"
#include "FastEngine/textureType.hpp"
#include <filesystem>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

#define FGE_ANIM_DEFAULT_TICKS 100

#define FGE_ANIM_DEFAULT FGE_ANIM_BAD
#define FGE_ANIM_BAD ""

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
    std::string _path;                          ///< The file path of the texture
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
    std::vector<fge::anim::AnimationFrame> _frames; ///< The vector of frames of the group
    std::string _groupName;                         ///< The name of the group
};

/**
 * \struct AnimationData
 * \ingroup animation
 * \brief Structure that contains the information of an animation
 */
struct AnimationData
{
    std::vector<fge::anim::AnimationGroup> _groups; ///< The vector of groups of the animation
    bool _valid;                                    ///< The validity of the animation
    std::filesystem::path _path;                    ///< The file path of the animation

    fge::anim::AnimationType _type; ///< The type of the animation
    fge::Vector2u _tilesetGridSize;  ///< The tileset grid size, only useful if the type is ANIM_TYPE_TILESET
    std::shared_ptr<fge::TextureType>
            _tilesetTexture;            ///< The tileset texture, only useful if the type is ANIM_TYPE_TILESET
    std::filesystem::path _tilesetPath; ///< The tileset texture path, only useful if the type is ANIM_TYPE_TILESET
};

using AnimationDataPtr = std::shared_ptr<fge::anim::AnimationData>;
using AnimationDataType = std::unordered_map<std::string, fge::anim::AnimationDataPtr>;

/**
 * \ingroup animation
 * @{
 */

/**
 * \brief Initialize the animation manager
 *
 * This function must be called before any attempt to use animations.
 * The animation manager is a global thread safe storage of animations.
 *
 * \warning The texture manager must be initialized before the animation manager.
 */
FGE_API void Init();
/**
 * \brief Check if the animation manager is initialized
 *
 * \return \b true if the animation manager is initialized, \b false otherwise
 */
FGE_API bool IsInit();
/**
 * \brief Un-initialize the animation manager
 */
FGE_API void Uninit();

/**
 * \brief Get the total number of loaded animations
 *
 * \return The total number of loaded animations
 */
FGE_API std::size_t GetAnimationSize();

/**
 * \brief Get the mutex of the animation manager
 *
 * \return The mutex of the animation manager
 */
FGE_API std::mutex& GetMutex();
/**
 * \brief Get the begin iterator of the animation manager
 *
 * \return The begin iterator of the animation manager
 */
FGE_API fge::anim::AnimationDataType::const_iterator GetCBegin();
/**
 * \brief Get the end iterator of the animation manager
 *
 * \return The end iterator of the animation manager
 */
FGE_API fge::anim::AnimationDataType::const_iterator GetCEnd();

/**
 * \brief Get the bad animation
 *
 * A bad animation is an animation that is not valid and is default returned when an animation is not found.
 *
 * \return The bad animation
 */
FGE_API const fge::anim::AnimationDataPtr& GetBadAnimation();
/**
 * \brief Get the animation with the given name
 *
 * \param name The name of the animation to get
 * \return The animation with the given name or the bad animation if not found
 */
FGE_API fge::anim::AnimationDataPtr GetAnimation(const std::string& name);

/**
 * \brief Check if the animation with the given name exist
 *
 * \param name The name of the animation to check
 * \return \b true if the animation exist, \b false otherwise
 */
FGE_API bool Check(const std::string& name);

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
FGE_API bool LoadFromFile(const std::string& name, std::filesystem::path path);
/**
 * \brief Unload the animation with the given name
 *
 * \param name The name of the animation to unload
 * \return \b true if the animation is unloaded, \b false otherwise
 */
FGE_API bool Unload(const std::string& name);
/**
 * \brief Unload all the animations
 */
FGE_API void UnloadAll();

/**
 * \brief Push a user handled animation to the animation manager
 *
 * \param name The name of the animation to push
 * \param data The animation to push
 * \return \b true if the animation is pushed, \b false otherwise
 */
FGE_API bool Push(const std::string& name, const fge::anim::AnimationDataPtr& data);

/**
 * @}
 */

} // namespace fge::anim

#endif // _FGE_ANIM_MANAGER_HPP_INCLUDED
