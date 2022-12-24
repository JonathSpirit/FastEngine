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

#include "FastEngine/manager/anim_manager.hpp"
#include "FastEngine/arbitraryJsonTypes.hpp"
#include "FastEngine/manager/texture_manager.hpp"
#include "FastEngine/vulkan/vulkanGlobal.hpp"

#include <fstream>

#include "json.hpp"

namespace fge::anim
{

namespace
{

fge::anim::AnimationDataPtr _dataAnimBad;
fge::anim::AnimationDataType _dataAnim;
std::mutex _dataMutex;

} // namespace

void Init()
{
    if (_dataAnimBad == nullptr)
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
    if (!inFile)
    {
        return false;
    }

    try
    {
        nlohmann::ordered_json inputJson;
        inFile >> inputJson;
        inFile.close();

        if (inputJson.empty())
        {
            return false;
        }

        std::string animTypeStr = inputJson["type"].get<std::string>();
        fge::anim::AnimationType animType;
        if (animTypeStr == "tileset")
        {
            animType = fge::anim::AnimationType::ANIM_TYPE_TILESET;
        }
        else if (animTypeStr == "separate")
        {
            animType = fge::anim::AnimationType::ANIM_TYPE_SEPARATE_FILES;
        }
        else
        {
            return false;
        }

        fge::anim::AnimationDataPtr buffAnimData = std::make_shared<fge::anim::AnimationData>();
        buffAnimData->_path = std::move(path);
        buffAnimData->_valid = true;
        buffAnimData->_type = animType;

        if (animType == fge::anim::AnimationType::ANIM_TYPE_TILESET)
        {
            buffAnimData->_tilesetPath = inputJson.value<std::filesystem::path>("tileset", {});

            fge::Surface buffSurface;
            if ( buffSurface.loadFromFile(buffAnimData->_tilesetPath) )
            {
                std::shared_ptr<fge::TextureType> buffTexture{new fge::TextureType{}};
                if (buffTexture->create(*vulkan::GlobalContext, buffSurface.get()))
                {
                    buffAnimData->_tilesetTexture = std::move(buffTexture);
                }
                else
                {
                    buffAnimData->_tilesetTexture = fge::texture::GetBadTexture()->_texture;
                }
            }
            else
            {
                buffAnimData->_tilesetTexture = fge::texture::GetBadTexture()->_texture;
            }

            buffAnimData->_tilesetGridSize = inputJson.value<fge::Vector2u>("gridSize", {0, 0});
        }

        nlohmann::ordered_json& inputJsonDataObject = inputJson["data"];
        if (!inputJsonDataObject.is_object())
        {
            return false;
        }

        buffAnimData->_groups.resize(inputJsonDataObject.size());

        std::size_t iGroup = 0;

        for (nlohmann::ordered_json::iterator itGroup = inputJsonDataObject.begin();
             itGroup != inputJsonDataObject.end(); ++itGroup)
        {
            nlohmann::ordered_json& jsonGroup = itGroup.value();

            fge::anim::AnimationGroup& tmpGroup = buffAnimData->_groups[iGroup];

            tmpGroup._groupName = itGroup.key();

            tmpGroup._frames.resize(jsonGroup.size());

            std::size_t iFrame = 0;

            for (nlohmann::ordered_json::iterator itFrame = jsonGroup.begin(); itFrame != jsonGroup.end(); ++itFrame)
            {
                nlohmann::ordered_json& jsonFrame = itFrame.value();

                fge::anim::AnimationFrame& tmpFrame = tmpGroup._frames[iFrame];

                switch (animType)
                {
                case AnimationType::ANIM_TYPE_TILESET:
                    tmpFrame._texturePosition = jsonFrame.value<fge::Vector2u>("position", {0, 0});
                    tmpFrame._texture = fge::texture::GetBadTexture()->_texture;
                    break;
                case AnimationType::ANIM_TYPE_SEPARATE_FILES:
                    tmpFrame._path = jsonFrame.value<std::string>("path", "");
                    break;
                }

                tmpFrame._ticks = jsonFrame.value<uint32_t>("ticks", FGE_ANIM_DEFAULT_TICKS);

                //Load texture
                if (animType != fge::anim::AnimationType::ANIM_TYPE_TILESET)
                {
                    fge::Surface buffSurface;
                    if ( buffSurface.loadFromFile(tmpFrame._path) )
                    {
                        std::shared_ptr<fge::TextureType> buffTexture{new fge::TextureType{}};
                        if (buffTexture->create(*vulkan::GlobalContext, buffSurface.get()))
                        {
                            tmpFrame._texture = std::move(buffTexture);
                        }
                        else
                        {
                            tmpFrame._texture = fge::texture::GetBadTexture()->_texture;
                        }
                    }
                    else
                    {
                        tmpFrame._texture = fge::texture::GetBadTexture()->_texture;
                    }
                }

                ++iFrame;
            }
            ++iGroup;
        }

        _dataAnim[name] = std::move(buffAnimData);
        return true;
    }
    catch (std::exception& e)
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

    for (auto& it: _dataAnim)
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
    if (fge::anim::Check(name))
    {
        return false;
    }

    _dataAnim.emplace(name, data);
    return true;
}

} // namespace fge::anim
