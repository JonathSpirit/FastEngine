#ifndef _FGE_ANIM_MANAGER_HPP_INCLUDED
#define _FGE_ANIM_MANAGER_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>

#include <SFML/Graphics/Texture.hpp>
#include <memory>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <filesystem>

#define FGE_ANIM_DEFAULT_TICKS 100

#define FGE_ANIM_DEFAULT FGE_ANIM_BAD
#define FGE_ANIM_BAD ""

namespace fge::anim
{

/**
 * \struct AnimationFrame
 * \ingroup animation
 * \brief Structure that contains the information of a frame of an animation
 */
struct AnimationFrame
{
    std::shared_ptr<sf::Texture> _texture; ///< The shared pointer texture of the frame
    std::string _path; ///< The file path of the texture

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
    std::string _groupName; ///< The name of the group
};

/**
 * \struct AnimationData
 * \ingroup animation
 * \brief Structure that contains the information of an animation
 */
struct AnimationData
{
    std::vector<fge::anim::AnimationGroup> _groups; ///< The vector of groups of the animation
    bool _valid; ///< The validity of the animation
    std::filesystem::path _path; ///< The file path of the animation
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
 * \return The animation with the given name
 */
FGE_API fge::anim::AnimationDataPtr GetAnimation(const std::string& name);

/**
 * \brief Check if the animation with the given name is valid
 *
 * \param name The name of the animation to check
 * \return \b true if the animation is valid, \b false otherwise
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
 *    "animationGroup1": [
 *    {
 *    "path": "path/to/the/texture1",
 *    "ticks": 10
 *    },
 *    {
 *    "path": "path/to/the/texture2",
 *    "ticks": 10
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

}//end fge::anim

#endif // _FGE_ANIM_MANAGER_HPP_INCLUDED
