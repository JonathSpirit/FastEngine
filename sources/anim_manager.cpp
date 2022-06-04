#include "FastEngine/anim_manager.hpp"
#include "FastEngine/texture_manager.hpp"

#include <fstream>

#include "json.hpp"

namespace fge::anim
{

namespace
{

fge::anim::AnimationDataPtr _dataAnimBad;
fge::anim::AnimationDataType _dataAnim;
std::mutex _dataMutex;

}//end

void Init()
{
    if (_dataAnimBad == nullptr )
    {
        _dataAnimBad = std::make_shared<fge::anim::AnimationData>();
        _dataAnimBad->_valid = false;
    }
}
bool IsInit()
{
    return _dataAnimBad != nullptr;
}
void Uninit()
{
    _dataAnim.clear();
    _dataAnimBad = nullptr;
}

std::size_t GetAnimationSize()
{
    std::lock_guard<std::mutex> lck(_dataMutex);
    return _dataAnim.size();
}

std::mutex& GetMutex()
{
    return _dataMutex;
}
fge::anim::AnimationDataType::const_iterator GetCBegin()
{
    return _dataAnim.cbegin();
}
fge::anim::AnimationDataType::const_iterator GetCEnd()
{
    return _dataAnim.cend();
}

const fge::anim::AnimationDataPtr& GetBadAnimation()
{
    return _dataAnimBad;
}
fge::anim::AnimationDataPtr GetAnimation(const std::string& name)
{
    if (name == FGE_ANIM_BAD)
    {
        return _dataAnimBad;
    }

    std::lock_guard<std::mutex> lck(_dataMutex);
    auto it = _dataAnim.find(name);

    if (it != _dataAnim.end())
    {
        return it->second;
    }
    return _dataAnimBad;
}

bool Check(const std::string& name)
{
    if (name == FGE_ANIM_BAD)
    {
        return false;
    }

    std::lock_guard<std::mutex> lck(_dataMutex);
    auto it = _dataAnim.find(name);

    return it != _dataAnim.end();
}

bool LoadFromFile(const std::string& name, std::filesystem::path path)
{
    if (name == FGE_ANIM_BAD)
    {
        return false;
    }

    std::lock_guard<std::mutex> lck(_dataMutex);
    auto it = _dataAnim.find(name);

    if (it != _dataAnim.end())
    {
        return false;
    }

    std::ifstream inFile(path);
    if ( !inFile )
    {
        return false;
    }

    try
    {
        nlohmann::ordered_json inputJson;
        inFile >> inputJson;
        inFile.close();

        if ( inputJson.empty() )
        {
            return false;
        }

        fge::anim::AnimationDataPtr buffAnimData = std::make_shared<fge::anim::AnimationData>();
        buffAnimData->_path = std::move(path);
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
#ifndef _FGE_DEF_SERVER
                std::shared_ptr<sf::Texture> buffTexture{ new sf::Texture() };
                if ( buffTexture->loadFromFile(tmpFrame._path) )
                {
                    tmpFrame._texture = std::move(buffTexture);
                }
                else
                {
                    tmpFrame._texture = fge::texture::GetBadTexture()->_texture;
                }
#else
                tmpFrame._texture = fge::texture::GetBadTexture()->_texture;
#endif //_FGE_DEF_SERVER

                ++iFrame;
            }
            ++iGroup;
        }

        _dataAnim[name] = std::move(buffAnimData);
        return true;
    }
    catch(std::exception& e)
    {
        inFile.close();
        return false;
    }
}
bool Unload(const std::string& name)
{
    if (name == FGE_ANIM_BAD)
    {
        return false;
    }

    std::lock_guard<std::mutex> lck(_dataMutex);
    auto it = _dataAnim.find(name);

    if (it != _dataAnim.end())
    {
        it->second->_valid = false;
        it->second->_groups.clear();
        _dataAnim.erase(it);
        return true;
    }
    return false;
}
void UnloadAll()
{
    std::lock_guard<std::mutex> lck(_dataMutex);

    for (auto& it : _dataAnim)
    {
        it.second->_valid = false;
        it.second->_groups.clear();
    }
    _dataAnim.clear();
}

bool Push(const std::string& name, const fge::anim::AnimationDataPtr& data)
{
    if (name == FGE_ANIM_BAD)
    {
        return false;
    }

    std::lock_guard<std::mutex> lck(_dataMutex);
    if ( fge::anim::Check(name) )
    {
        return false;
    }

    _dataAnim.emplace(name, data);
    return true;
}

}//end fge::anim
