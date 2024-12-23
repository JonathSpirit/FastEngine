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

#ifndef _FGE_TEXTURE_MANAGER_HPP_INCLUDED
#define _FGE_TEXTURE_MANAGER_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"

#include "FastEngine/graphic/C_surface.hpp"
#include "FastEngine/manager/C_baseManager.hpp"
#include "FastEngine/textureType.hpp"
#include <vector>

#define FGE_TEXTURE_BAD FGE_MANAGER_BAD
#define FGE_TEXTURE_BAD_W 32
#define FGE_TEXTURE_BAD_H 32
#define FGE_TEXTURE_BAD_COLOR_1 fge::Color::Black
#define FGE_TEXTURE_BAD_COLOR_2 fge::Color::Magenta

namespace fge::texture
{

struct DataBlock : manager::BaseDataBlock<TextureType>
{
    inline void unload() override { this->_group.clear(); }

    std::vector<DataPointer> _group;
};

/**
 * \class TextureManager
 * \ingroup graphics
 * \brief Manage textures
 *
 * This class is used to manage textures. It can load textures from files or surfaces.
 * On initialization, a bad texture is created.
 * This texture is used when a texture is not found in order to always return a "valid" texture.
 */
class FGE_API TextureManager : public manager::BaseManager<TextureType, DataBlock>
{
public:
    using BaseManager::BaseManager;

    bool initialize() override;
    void uninitialize() override;

    /**
     * \brief Load a texture from a surface
     *
     * \param name The name of the texture to load
     * \param surface The surface to load
     * \return \b true if the texture was loaded, \b false otherwise
     */
    bool loadFromSurface(std::string_view name, fge::Surface const& surface);
    /**
     * \brief Load a texture from a file
     *
     * \param name The name of the texture to load
     * \param path The path of the file to load
     * \return \b true if the texture was loaded, \b false otherwise
     */
    bool loadFromFile(std::string_view name, std::filesystem::path const& path);
    /**
     * \brief Load a texture from a surface and add it to a group
     *
     * \param name The name of the texture to load
     * \param surface The surface to load
     * \return \b true if the texture was loaded, \b false otherwise
     */
    bool loadToGroupFromSurface(std::string_view name, fge::Surface const& surface) const;
};

/**
 * \ingroup managers
 * \brief The global texture manager
 */
FGE_API extern TextureManager gManager;

} // namespace fge::texture

#endif // _FGE_TEXTURE_MANAGER_HPP_INCLUDED
