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

#include "FastEngine/manager/font_manager.hpp"

#include "ft2build.h"
#include FT_FREETYPE_H

#include <atomic>

namespace fge::font
{

namespace
{

FT_Library gFreetypeLibrary = nullptr;
std::atomic<unsigned int> gCounter = 0;

} // namespace

bool FontManager::initialize()
{
    if (this->isInitialized())
    {
        return true;
    }

    if (gFreetypeLibrary == nullptr)
    {
        if (FT_Init_FreeType(&gFreetypeLibrary) != 0)
        {
            return false;
        }
    }

    this->_g_badElement = std::make_shared<DataBlockType>();
    this->_g_badElement->_ptr = std::make_shared<DataType>();
    this->_g_badElement->_valid = false;

    ++gCounter;
    return true;
}

bool FontManager::isInitialized()
{
    return this->_g_badElement != nullptr;
}

void FontManager::uninitialize()
{
    if (!this->isInitialized())
    {
        return;
    }

    this->unloadAll();
    this->_g_badElement.reset();

    if (--gCounter == 0)
    {
        FT_Done_FreeType(gFreetypeLibrary);
        gFreetypeLibrary = nullptr;
    }
}

bool FontManager::loadFromFile(std::string_view name, std::filesystem::path const& path)
{
    if (name.empty())
    {
        return false;
    }

    auto newFont = std::make_shared<DataType>();

    if (!newFont->loadFromFile(path.string()))
    {
        return false;
    }

    DataBlockPointer block = std::make_shared<DataBlockType>();
    block->_ptr = std::move(newFont);
    block->_valid = true;
    block->_path = path;

    return this->push(name, std::move(block));
}

FontManager gManager;

void* GetFreetypeLibrary()
{
    return gFreetypeLibrary;
}

} // namespace fge::font
