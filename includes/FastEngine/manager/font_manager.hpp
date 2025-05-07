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

#ifndef _FGE_FONT_MANAGER_HPP_INCLUDED
#define _FGE_FONT_MANAGER_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"

#include "FastEngine/graphic/C_ftFont.hpp"
#include "FastEngine/manager/C_baseManager.hpp"

#define FGE_FONT_BAD FGE_MANAGER_BAD

namespace fge::font
{

struct DataBlock : manager::BaseDataBlock<FreeTypeFont>
{};

/**
 * \class FontManager
 * \ingroup graphics
 * \brief Manage fonts
 *
 * \see TextureManager
 */
class FGE_API FontManager : public manager::BaseManager<FreeTypeFont, DataBlock>
{
public:
    using BaseManager::BaseManager;

    bool initialize() override;
    void uninitialize() override;

    /**
     * \brief Load a font from a file
     *
     * \param name The name of the font to load
     * \param path The path of the font to load
     * \return \b true if the font was loaded, \b false otherwise
     */
    bool loadFromFile(std::string_view name, std::filesystem::path const& path);
};

/**
 * \ingroup managers
 * \brief The global font manager
 */
FGE_API extern FontManager gManager;

FGE_API void* GetFreetypeLibrary();

} // namespace fge::font

#endif // _FGE_FONT_MANAGER_HPP_INCLUDED
