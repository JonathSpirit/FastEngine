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

#include "FastEngine/manager/anim_manager.hpp"
#include "FastEngine/arbitraryJsonTypes.hpp"
#include "FastEngine/extra/extra_function.hpp"
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
    std::scoped_lock<std::mutex> const lck(_dataMutex);
    return _dataAnim.size();
}

fge::AccessLock<std::mutex> AcquireLock()
{
    return fge::AccessLock<std::mutex>{_dataMutex};
}
fge::anim::AnimationDataType::const_iterator IteratorBegin(fge::AccessLock<std::mutex> const& lock)
{
    lock.throwIfDifferent(_dataMutex);
    return _dataAnim.cbegin();
}
fge::anim::AnimationDataType::const_iterator IteratorEnd(fge::AccessLock<std::mutex> const& lock)
{
    lock.throwIfDifferent(_dataMutex);
    return _dataAnim.cend();
}

fge::anim::AnimationDataPtr const& GetBadAnimation()
{
    return _dataAnimBad;
}
fge::anim::AnimationDataPtr GetAnimation(std::string const& name)
{
    if (name == FGE_ANIM_BAD)
    {
        return _dataAnimBad;
    }

    std::scoped_lock<std::mutex> const lck(_dataMutex);
    auto it = _dataAnim.find(name);

    if (it != _dataAnim.end())
    {
        return it->second;
    }
    return _dataAnimBad;
}

bool Check(std::string const& name)
{
    if (name == FGE_ANIM_BAD)
    {
        return false;
    }

    std::scoped_lock<std::mutex> const lck(_dataMutex);
    auto it = _dataAnim.find(name);

    return it != _dataAnim.end();
}

bool LoadFromFile(std::string const& name, std::filesystem::path path)
{
    if (name == FGE_ANIM_BAD)
    {
        return false;
    }

    std::scoped_lock<std::mutex> const lck(_dataMutex);
    auto it = _dataAnim.find(name);

    if (it != _dataAnim.end())
    {
        return false;
    }

    nlohmann::ordered_json inputJson;

    if (!fge::LoadOrderedJsonFromFile(path, inputJson) || !inputJson.is_object())
    {
        return false;
    }

    auto itType = inputJson.find("type");
    if (itType == inputJson.end() || !itType->is_string())
    {
        return false;
    }

    std::string typeString = itType->get<std::string>();
    fge::anim::AnimationType type;
    if (typeString == "tileset")
    {
        type = fge::anim::AnimationType::ANIM_TYPE_TILESET;
    }
    else if (typeString == "separate")
    {
        type = fge::anim::AnimationType::ANIM_TYPE_SEPARATE_FILES;
    }
    else
    {
        return false;
    }

    fge::anim::AnimationDataPtr buffAnimData = std::make_shared<fge::anim::AnimationData>();
    buffAnimData->_path = std::move(path);
    buffAnimData->_valid = true;
    buffAnimData->_type = type;

    if (type == fge::anim::AnimationType::ANIM_TYPE_TILESET)
    {
        auto itPath = inputJson.find("tileset");
        if (itPath == inputJson.end() || !itPath->is_string())
        {
            return false;
        }

        buffAnimData->_tilesetPath =
                fge::MakeRelativePathToBasePathIfExist(buffAnimData->_path, itPath->get<std::filesystem::path>());
        buffAnimData->_tilesetTexture = fge::texture::gManager.getBadElement()->_ptr;

        fge::Surface textureSurface;
        if (textureSurface.loadFromFile(buffAnimData->_tilesetPath))
        {
#ifdef FGE_DEF_SERVER
            buffAnimData->_tilesetTexture = std::make_shared<fge::TextureType>(std::move(textureSurface));
#else
            auto texture = std::make_shared<fge::TextureType>(vulkan::GetActiveContext());
            if (texture->create(textureSurface.get()))
            {
                buffAnimData->_tilesetTexture = std::move(texture);
            }
#endif // FGE_DEF_SERVER
        }

        buffAnimData->_tilesetGridSize = inputJson.value<fge::Vector2u>("gridSize", {0, 0});
    }

    auto itData = inputJson.find("data");
    if (itData == inputJson.end() || !itData->is_object())
    {
        return false;
    }

    buffAnimData->_groups.reserve(itData->size());

    for (auto const& [dataKey, dataValue]: itData->items())
    {
        if (!dataValue.is_array())
        {
            continue;
        }

        fge::anim::AnimationGroup group{};

        group._groupName = dataKey;
        group._frames.reserve(dataValue.size());

        for (auto const& jsonFrame: dataValue)
        {
            if (!jsonFrame.is_object())
            {
                continue;
            }

            fge::anim::AnimationFrame frame{};
            frame._texture = fge::texture::gManager.getBadElement()->_ptr;

            switch (type)
            {
            case AnimationType::ANIM_TYPE_TILESET:
                std::numeric_limits<fge::Vector2u>::max();
                frame._texturePosition =
                        jsonFrame.value<fge::Vector2u>("position", FGE_NUMERIC_LIMITS_VECTOR_MAX(fge::Vector2u));
                break;
            case AnimationType::ANIM_TYPE_SEPARATE_FILES:
                frame._path = fge::MakeRelativePathToBasePathIfExist(
                        buffAnimData->_path, jsonFrame.value<std::filesystem::path>("path", {}));
                {
                    fge::Surface textureSurface;
                    if (textureSurface.loadFromFile(frame._path))
                    {
#ifdef FGE_DEF_SERVER
                        frame._texture = std::make_shared<fge::TextureType>(std::move(textureSurface));
#else
                        auto texture = std::make_shared<fge::TextureType>(vulkan::GetActiveContext());
                        if (texture->create(textureSurface.get()))
                        {
                            frame._texture = std::move(texture);
                        }
#endif //FGE_DEF_SERVER
                    }
                }
                break;
            }

            frame._ticks = jsonFrame.value<uint32_t>("ticks", FGE_ANIM_DEFAULT_TICKS);
            group._frames.push_back(std::move(frame));
        }
        buffAnimData->_groups.push_back(std::move(group));
    }

    _dataAnim[name] = std::move(buffAnimData);
    return true;
}
bool Unload(std::string const& name)
{
    if (name == FGE_ANIM_BAD)
    {
        return false;
    }

    std::scoped_lock<std::mutex> const lck(_dataMutex);
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
    std::scoped_lock<std::mutex> const lck(_dataMutex);

    for (auto& it: _dataAnim)
    {
        it.second->_valid = false;
        it.second->_groups.clear();
    }
    _dataAnim.clear();
}

bool Push(std::string const& name, fge::anim::AnimationDataPtr const& data)
{
    if (name == FGE_ANIM_BAD)
    {
        return false;
    }

    std::scoped_lock<std::mutex> const lck(_dataMutex);
    if (fge::anim::Check(name))
    {
        return false;
    }

    _dataAnim.emplace(name, data);
    return true;
}

} // namespace fge::anim
