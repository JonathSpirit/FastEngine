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

void FGE_API Init();
bool FGE_API IsInit();
void FGE_API Uninit();

std::size_t FGE_API GetAnimationSize();

std::mutex& GetMutex();
fge::anim::AnimationDataType::const_iterator FGE_API GetCBegin();
fge::anim::AnimationDataType::const_iterator FGE_API GetCEnd();

const fge::anim::AnimationDataPtr& FGE_API GetBadAnimation();
fge::anim::AnimationDataPtr FGE_API GetAnimation(const std::string& name);

bool FGE_API Check(const std::string& name);

bool FGE_API LoadFromFile(const std::string& name, const std::string& path);
bool FGE_API Unload(const std::string& name);
void FGE_API UnloadAll();

bool FGE_API Push(const std::string& name, const fge::anim::AnimationDataPtr& data);

}//end anim
}//end fge

#endif // _FGE_ANIM_MANAGER_HPP_INCLUDED
