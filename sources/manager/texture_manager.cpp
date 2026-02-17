/*
 * Copyright 2026 Guillaume Guillet
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

#include "FastEngine/manager/texture_manager.hpp"
#include "FastEngine/vulkan/vulkanGlobal.hpp"
#include "SDL_image.h"

namespace fge::texture
{

namespace
{

std::mutex gSDLImageHandlerMutex;
unsigned int gSDLImageHandlerCounter = 0;

} // namespace

bool TextureManager::initialize()
{
    if (this->isInitialized())
    {
        return true;
    }

    gSDLImageHandlerMutex.lock();
    if (gSDLImageHandlerCounter == 0)
    {
        IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF | IMG_INIT_WEBP | IMG_INIT_JXL | IMG_INIT_AVIF);
    }
    ++gSDLImageHandlerCounter;
    gSDLImageHandlerMutex.unlock();

    fge::Surface badSurface;

    if (!badSurface.create(FGE_TEXTURE_BAD_W, FGE_TEXTURE_BAD_H, FGE_TEXTURE_BAD_COLOR_1))
    {
        return false;
    }

    for (int y = 0; y < FGE_TEXTURE_BAD_H / 2; ++y)
    {
        for (int x = 0; x < FGE_TEXTURE_BAD_W / 2; ++x)
        {
            badSurface.setPixel(x, y, FGE_TEXTURE_BAD_COLOR_2);
        }
    }
    for (int y = FGE_TEXTURE_BAD_H / 2; y < FGE_TEXTURE_BAD_H; ++y)
    {
        for (int x = FGE_TEXTURE_BAD_W / 2; x < FGE_TEXTURE_BAD_W; ++x)
        {
            badSurface.setPixel(x, y, FGE_TEXTURE_BAD_COLOR_2);
        }
    }

    this->_g_badElement = std::make_shared<DataBlockType>();
#ifdef FGE_DEF_SERVER
    this->_g_badElement->_ptr = std::make_shared<DataType>(badSurface);
#else
    this->_g_badElement->_ptr = std::make_shared<DataType>(vulkan::GetActiveContext());
    this->_g_badElement->_ptr->create(badSurface.get());
#endif //FGE_DEF_SERVER
    this->_g_badElement->_valid = false;
    return true;
}

void TextureManager::uninitialize()
{
    BaseManager::uninitialize();
    gSDLImageHandlerMutex.lock();
    if (--gSDLImageHandlerCounter == 0)
    {
        IMG_Quit();
    }
    gSDLImageHandlerMutex.unlock();
}

bool TextureManager::loadFromSurface(std::string_view name, fge::Surface const& surface)
{
    if (name.empty() || surface.get() == nullptr)
    {
        return false;
    }

#ifdef FGE_DEF_SERVER
    auto tmpTexture = std::make_shared<DataType>(surface);
#else
    auto tmpTexture = std::make_shared<DataType>(vulkan::GetActiveContext());

    if (!tmpTexture->create(surface.get()))
    {
        return false;
    }
#endif //FGE_DEF_SERVER

    DataBlockPointer block = std::make_shared<DataBlockType>();
    block->_ptr = std::move(tmpTexture);
    block->_valid = true;

    return this->push(name, std::move(block));
}

bool TextureManager::loadFromFile(std::string_view name, std::filesystem::path const& path)
{
    fge::Surface tmpSurface;

    if (!tmpSurface.loadFromFile(path))
    {
        return false;
    }

    return this->loadFromSurface(name, tmpSurface);
}

bool TextureManager::loadToGroupFromSurface(std::string_view name, fge::Surface const& surface) const
{
    auto data = this->getElement(name);
    if (!data)
    {
        return false;
    }

#ifdef FGE_DEF_SERVER
    auto tmpTexture = std::make_shared<DataType>(surface);
#else
    auto tmpTexture = std::make_shared<DataType>(vulkan::GetActiveContext());

    if (!tmpTexture->create(surface.get()))
    {
        return false;
    }
#endif //FGE_DEF_SERVER

    data->_group.push_back(std::move(tmpTexture));
    return true;
}

TextureManager gManager;

} // namespace fge::texture
