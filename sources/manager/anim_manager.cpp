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

#include "FastEngine/manager/anim_manager.hpp"
#include "FastEngine/arbitraryJsonTypes.hpp"
#include "FastEngine/extra/extra_function.hpp"
#include "FastEngine/manager/texture_manager.hpp"
#include "FastEngine/vulkan/vulkanGlobal.hpp"

#include "json.hpp"

namespace fge::anim
{

bool AnimationManager::initialize()
{
    if (this->isInitialized())
    {
        return true;
    }

    this->_g_badElement = std::make_shared<DataBlockType>();
    this->_g_badElement->_ptr = std::make_shared<DataType>();
    this->_g_badElement->_ptr->_type = AnimationType::ANIM_TYPE_SEPARATE_FILES;
    this->_g_badElement->_valid = false;
    return true;
}

bool AnimationManager::loadFromFile(std::string_view name, std::filesystem::path const& path)
{
    if (name.empty())
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
    AnimationType type;
    if (typeString == "tileset")
    {
        type = AnimationType::ANIM_TYPE_TILESET;
    }
    else if (typeString == "separate")
    {
        type = AnimationType::ANIM_TYPE_SEPARATE_FILES;
    }
    else
    {
        return false;
    }

    DataBlockPointer block = std::make_shared<DataBlockType>();
    block->_path = path;
    block->_valid = true;
    block->_ptr = std::make_shared<DataType>();
    block->_ptr->_type = type;

    auto& animData = *block->_ptr.get();

    if (type == AnimationType::ANIM_TYPE_TILESET)
    {
        auto itPath = inputJson.find("tileset");
        if (itPath == inputJson.end() || !itPath->is_string())
        {
            return false;
        }

        animData._tilesetPath =
                fge::MakeRelativePathToBasePathIfExist(block->_path, itPath->get<std::filesystem::path>());
        animData._tilesetTexture = fge::texture::gManager.getBadElement()->_ptr;

        fge::Surface textureSurface;
        if (textureSurface.loadFromFile(animData._tilesetPath))
        {
#ifdef FGE_DEF_SERVER
            animData._tilesetTexture = std::make_shared<fge::TextureType>(std::move(textureSurface));
#else
            auto texture = std::make_shared<fge::TextureType>(vulkan::GetActiveContext());
            if (texture->create(textureSurface.get()))
            {
                animData._tilesetTexture = std::move(texture);
            }
#endif // FGE_DEF_SERVER
        }

        animData._tilesetGridSize = inputJson.value<fge::Vector2u>("gridSize", {0, 0});
    }

    auto itData = inputJson.find("data");
    if (itData == inputJson.end() || !itData->is_object())
    {
        return false;
    }

    animData._groups.reserve(itData->size());

    for (auto const& [dataKey, dataValue]: itData->items())
    {
        if (!dataValue.is_array())
        {
            continue;
        }

        AnimationGroup group{};

        group._groupName = dataKey;
        group._frames.reserve(dataValue.size());

        for (auto const& jsonFrame: dataValue)
        {
            if (!jsonFrame.is_object())
            {
                continue;
            }

            AnimationFrame frame{};
            frame._texture = fge::texture::gManager.getBadElement()->_ptr;

            switch (type)
            {
            case AnimationType::ANIM_TYPE_TILESET:
                frame._texturePosition =
                        jsonFrame.value<fge::Vector2u>("position", FGE_NUMERIC_LIMITS_VECTOR_MAX(fge::Vector2u));
                break;
            case AnimationType::ANIM_TYPE_SEPARATE_FILES:
                frame._path = fge::MakeRelativePathToBasePathIfExist(
                        block->_path, jsonFrame.value<std::filesystem::path>("path", {}));
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
        animData._groups.push_back(std::move(group));
    }

    return this->push(name, std::move(block));
}

AnimationManager gManager;

} // namespace fge::anim
