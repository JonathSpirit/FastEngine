#include "FastEngine/anim_manager.hpp"
#include "FastEngine/texture_manager.hpp"

#include <fstream>
#include <iomanip>

#include "json.hpp"

namespace fge
{
namespace anim
{

namespace
{

fge::anim::AnimationDataPtr __dataAnimBad;
fge::anim::AnimationDataType __dataAnim;
std::mutex __dataMutex;

}//end

void FGE_API Init()
{
    if ( __dataAnimBad == nullptr )
    {
        __dataAnimBad = std::make_shared<fge::anim::AnimationData>();
        __dataAnimBad->_valid = false;
    }
}
bool FGE_API IsInit()
{
    return __dataAnimBad != nullptr;
}
void FGE_API Uninit()
{
    __dataAnim.clear();
    __dataAnimBad = nullptr;
}

std::size_t FGE_API GetAnimationSize()
{
    std::lock_guard<std::mutex> lck(__dataMutex);
    return __dataAnim.size();
}

std::mutex& GetMutex()
{
    return __dataMutex;
}
fge::anim::AnimationDataType::const_iterator FGE_API GetCBegin()
{
    return __dataAnim.cbegin();
}
fge::anim::AnimationDataType::const_iterator FGE_API GetCEnd()
{
    return __dataAnim.cend();
}

const fge::anim::AnimationDataPtr& FGE_API GetBadAnimation()
{
    return __dataAnimBad;
}
fge::anim::AnimationDataPtr FGE_API GetAnimation(const std::string& name)
{
    if (name == FGE_ANIM_BAD)
    {
        return __dataAnimBad;
    }

    std::lock_guard<std::mutex> lck(__dataMutex);
    fge::anim::AnimationDataType::iterator it = __dataAnim.find(name);

    if (it != __dataAnim.end())
    {
        return it->second;
    }
    return __dataAnimBad;
}

bool FGE_API Check(const std::string& name)
{
    if (name == FGE_ANIM_BAD)
    {
        return false;
    }

    std::lock_guard<std::mutex> lck(__dataMutex);
    fge::anim::AnimationDataType::iterator it = __dataAnim.find(name);

    if (it != __dataAnim.end())
    {
        return true;
    }
    return false;
}

bool FGE_API LoadFromFile(const std::string& name, const std::string& path)
{
    if (name == FGE_ANIM_BAD)
    {
        return false;
    }

    std::lock_guard<std::mutex> lck(__dataMutex);
    fge::anim::AnimationDataType::iterator it = __dataAnim.find(name);

    if (it != __dataAnim.end())
    {
        return false;
    }

    std::ifstream inFile(path);
    if ( !inFile.is_open() )
    {
        return false;
    }

    try
    {
        nlohmann::ordered_json inputJson;
        inFile >> inputJson;
        inFile.close();

        if ( !inputJson.size() )
        {
            return false;
        }

        fge::anim::AnimationDataPtr buffAnimData = std::make_shared<fge::anim::AnimationData>();
        buffAnimData->_path = path;
        buffAnimData->_valid = true;

        buffAnimData->_groups.resize( inputJson.size() );

        std::size_t iGroup = 0;

        for ( nlohmann::ordered_json::iterator itGroup=inputJson.begin(); itGroup!=inputJson.end(); ++itGroup )
        {
            nlohmann::ordered_json& jsonGroup = itGroup.value();

            fge::anim::AnimationGroup& tmpGroup = buffAnimData->_groups[iGroup];

            tmpGroup._groupName = itGroup.key();

            tmpGroup._frames.resize( jsonGroup.size() );

            std::size_t iFrame = 0;

            for ( nlohmann::ordered_json::iterator itFrame=jsonGroup.begin(); itFrame!=jsonGroup.end(); ++itFrame )
            {
                nlohmann::ordered_json& jsonFrame = itFrame.value();

                fge::anim::AnimationFrame& tmpFrame = tmpGroup._frames[iFrame];

                tmpFrame._path = jsonFrame.value<std::string>("path", "");
                tmpFrame._ticks = jsonFrame.value<uint32_t>("ticks", FGE_ANIM_DEFAULT_TICKS);

                //Load texture
                sf::Texture* buffTexture = new sf::Texture();
                if ( buffTexture->loadFromFile(tmpFrame._path) )
                {
                    tmpFrame._texture = std::shared_ptr<sf::Texture>(buffTexture);
                }
                else
                {
                    delete buffTexture;
                    tmpFrame._texture = fge::texture::GetBadTexture()->_texture;
                }

                ++iFrame;
            }
            ++iGroup;
        }

        __dataAnim[name] = std::move(buffAnimData);
        return true;
    }
    catch(std::exception& e)
    {
        inFile.close();
        return false;
    }
}
bool FGE_API Unload(const std::string& name)
{
    if (name == FGE_ANIM_BAD)
    {
        return false;
    }

    std::lock_guard<std::mutex> lck(__dataMutex);
    fge::anim::AnimationDataType::iterator it = __dataAnim.find(name);

    if (it != __dataAnim.end())
    {
        it->second->_valid = false;
        it->second->_groups.clear();
        __dataAnim.erase(it);
        return true;
    }
    return false;
}
void FGE_API UnloadAll()
{
    std::lock_guard<std::mutex> lck(__dataMutex);

    for (fge::anim::AnimationDataType::iterator it=__dataAnim.begin(); it!=__dataAnim.end(); ++it)
    {
        it->second->_valid = false;
        it->second->_groups.clear();
    }
    __dataAnim.clear();
}

bool FGE_API Push(const std::string& name, const fge::anim::AnimationDataPtr& data)
{
    if (name == FGE_ANIM_BAD)
    {
        return false;
    }

    std::lock_guard<std::mutex> lck(__dataMutex);
    if ( fge::anim::Check(name) )
    {
        return false;
    }

    __dataAnim.emplace(name, data);
    return true;
}

}//end anim
}//end fge
