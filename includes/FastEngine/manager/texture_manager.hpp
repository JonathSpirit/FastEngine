/*
 * Copyright 2023 Guillaume Guillet
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

#include "FastEngine/fastengine_extern.hpp"

#include "FastEngine/graphic/C_surface.hpp"
#include "FastEngine/textureType.hpp"
#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#define FGE_TEXTURE_DEFAULT FGE_TEXTURE_BAD
#define FGE_TEXTURE_BAD ""

namespace fge::texture
{

/**
 * \struct TextureData
 * \ingroup graphics
 * \brief Structure that safely contains the texture data with his path and validity
 */
struct TextureData
{
    std::shared_ptr<fge::TextureType> _texture;
    bool _valid;
    std::filesystem::path _path;
};

using TextureDataPtr = std::shared_ptr<fge::texture::TextureData>;
using TextureDataType = std::unordered_map<std::string, fge::texture::TextureDataPtr>;

/**
 * \ingroup graphics
 * @{
 */

/**
 * \brief Initialize the texture manager
 *
 * A bad texture is created with this function, it is used when a texture is not found.
 */
FGE_API void Init();
/**
 * \brief Check if the texture manager is initialized
 *
 * \return \b true if the texture manager is initialized, \b false otherwise
 */
FGE_API bool IsInit();
/**
 * \brief Un-initialize the texture manager
 */
FGE_API void Uninit();

/**
 * \brief Get the total number of loaded textures
 *
 * \return The total number of loaded textures
 */
FGE_API std::size_t GetTextureSize();

/**
 * \brief Acquire a unique lock, with the texture manager mutex
 *
 * In order to use iterators, you have to acquire a unique lock from this
 * function.
 * The lock is not differed and will lock the mutex.
 *
 * \return A unique lock bound to this mutex
 */
FGE_API std::unique_lock<std::mutex> AcquireLock();
/**
 * \brief Get the begin iterator of the texture manager
 *
 * You have to provide a valid reference to a unique lock acquire with
 * the function AcquireLock().
 * This function will throw if one of this is not respected :
 * - The lock does not owned the associated mutex.
 * - The mutex pointer of the lock does not correspond to this mutex.
 *
 * \param lock A unique lock bound to this mutex
 * \return The begin iterator of the texture manager
 */
FGE_API fge::texture::TextureDataType::const_iterator IteratorBegin(std::unique_lock<std::mutex> const& lock);
/**
 * \brief Get the end iterator of the texture manager
 *
 * \see fge::texture::IteratorBegin()
 *
 * \param lock A unique lock bound to this mutex
 * \return The begin iterator of the texture manager
 */
FGE_API fge::texture::TextureDataType::const_iterator IteratorEnd(std::unique_lock<std::mutex> const& lock);

/**
 * \brief Get the bad texture
 *
 * \return The bad texture
 */
FGE_API fge::texture::TextureDataPtr const& GetBadTexture();
/**
 * \brief Get the texture with the given name
 *
 * \param name The name of the texture to get
 * \return The texture with the given name or the bad texture if not found
 */
FGE_API fge::texture::TextureDataPtr GetTexture(std::string_view name);

/**
 * \brief Check if the texture with the given name exist
 *
 * \param name The name of the texture to check
 * \return \b true if the texture exist, \b false otherwise
 */
FGE_API bool Check(std::string_view name);

/**
 * \brief Load a texture from an surface
 *
 * \param name The name of the texture to load
 * \param surface The surface to load
 * \return \b true if the texture was loaded, \b false otherwise
 */
FGE_API bool LoadFromSurface(std::string_view name, fge::Surface const& surface);
/**
 * \brief Load a texture from a file
 *
 * \param name The name of the texture to load
 * \param path The path of the file to load
 * \return \b true if the texture was loaded, \b false otherwise
 */
FGE_API bool LoadFromFile(std::string_view name, std::filesystem::path path);
/**
 * \brief Unload the texture with the given name
 *
 * \param name The name of the texture to unload
 * \return \b true if the texture was unloaded, \b false otherwise
 */
FGE_API bool Unload(std::string_view name);
/**
 * \brief Unload all textures
 */
FGE_API void UnloadAll();

/**
 * \brief Add a user handled texture
 *
 * \param name The name of the texture to add
 * \param data The texture data to add
 * \return \b true if the texture was added, \b false otherwise
 */
FGE_API bool Push(std::string_view name, fge::texture::TextureDataPtr const& data);

/**
 * @}
 */

} // namespace fge::texture

#endif // _FGE_TEXTURE_MANAGER_HPP_INCLUDED
