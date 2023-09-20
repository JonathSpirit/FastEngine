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

#ifndef _FGE_C_TEXTURE_HPP_INCLUDED
#define _FGE_C_TEXTURE_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "C_vector.hpp"
#include "FastEngine/manager/texture_manager.hpp"
#include "json.hpp"

namespace fge
{

namespace net
{

class Packet;

} // namespace net

/**
 * \class Texture
 * \ingroup graphics
 * \brief This class is a wrapper for the texture manger to allow easy manipulation
 */
class FGE_API Texture
{
public:
    Texture();
    /**
     * \brief Get the texture data by its name
     *
     * \param name The name of the loaded texture
     */
    Texture(std::string name);
    Texture(char const* name);
    /**
     * \brief Copy a custom texture data pointer.
     *
     * \param data The custom texture data pointer
     */
    Texture(fge::texture::TextureDataPtr data);

    /**
     * \brief Clear the texture data
     *
     * This function clear the texture data by setting it to the default texture.
     */
    void clear();
    /**
     * \brief Reload the texture data from the same name
     */
    void refresh();

    /**
     * \brief Check if the texture is valid (not unloaded)
     *
     * \return \b true if the texture is valid, \b false otherwise
     */
    [[nodiscard]] bool valid() const;

    /**
     * \brief Get the texture size
     *
     * \return The texture size
     */
    [[nodiscard]] fge::Vector2u getTextureSize() const;

    /**
     * \brief Get the texture data
     *
     * \return The texture data
     */
    [[nodiscard]] fge::texture::TextureDataPtr const& getData() const;
    /**
     * \brief Get the name of the texture
     *
     * \return The name of the texture
     */
    [[nodiscard]] std::string const& getName() const;

    /**
     * \brief Get the texture data by its name
     *
     * \param name The name of the loaded texture
     */
    fge::Texture& operator=(std::string name);
    fge::Texture& operator=(char const* name);
    /**
     * \brief Copy a custom texture data pointer.
     *
     * \param data The custom texture data pointer
     */
    fge::Texture& operator=(fge::texture::TextureDataPtr data);

    /**
     * \brief Retrieve the internal texture type pointer
     *
     * \warning Will never be \b nullptr if the texture manager was correctly initialized.
     *
     * \see fge::TextureType
     *
     * \return The texture type pointer
     */
    [[nodiscard]] fge::TextureType* retrieve();
    [[nodiscard]] fge::TextureType const* retrieve() const;

private:
    fge::texture::TextureDataPtr g_data;
    std::string g_name;
};

FGE_API fge::net::Packet const& operator>>(fge::net::Packet const& pck, fge::Texture& data);
FGE_API fge::net::Packet& operator<<(fge::net::Packet& pck, fge::Texture const& data);

FGE_API void to_json(nlohmann::json& j, fge::Texture const& p);
FGE_API void from_json(nlohmann::json const& j, fge::Texture& p);

} // namespace fge

#endif // _FGE_C_TEXTURE_HPP_INCLUDED
