#ifndef _FGE_ANIM_MANAGER_HPP_INCLUDED
#define _FGE_ANIM_MANAGER_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>

#include <SFML/Graphics/Texture.hpp>
#include <memory>
#include <vector>
#include <unordered_map>
#include <mutex>

#define FGE_ANIM_DEFAULT_TICKS 100

#define FGE_ANIM_DEFAULT FGE_ANIM_BAD
#define FGE_ANIM_BAD ""

namespace fge
{
namespace anim
{

struct AnimationFrame
{
    std::shared_ptr<sf::Texture> _texture;
    std::string _path;

    uint32_t _ticks;
};

struct AnimationGroup
{
    std::vector<fge::anim::AnimationFrame> _frames;
    std::string _groupName;
};

struct AnimationData
{
    std::vector<fge::anim::AnimationGroup> _groups;
    bool _valid;
    std::string _path;
};

using AnimationDataPtr = std::shared_ptr<fge::anim::AnimationData>;
using AnimationDataType = std::unordered_map<std::string, fge::anim::AnimationDataPtr>;

FGE_API void Init();
FGE_API bool IsInit();
FGE_API void Uninit();

FGE_API std::size_t GetAnimationSize();

FGE_API std::mutex& GetMutex();
FGE_API fge::anim::AnimationDataType::const_iterator GetCBegin();
FGE_API fge::anim::AnimationDataType::const_iterator GetCEnd();

FGE_API const fge::anim::AnimationDataPtr& GetBadAnimation();
FGE_API fge::anim::AnimationDataPtr GetAnimation(const std::string& name);

FGE_API bool Check(const std::string& name);

FGE_API bool LoadFromFile(const std::string& name, const std::string& path);
FGE_API bool Unload(const std::string& name);
FGE_API void UnloadAll();

FGE_API bool Push(const std::string& name, const fge::anim::AnimationDataPtr& data);

}//end anim
}//end fge

#endif // _FGE_ANIM_MANAGER_HPP_INCLUDED
