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

#ifndef _FGE_C_TEXTURE_HPP_INCLUDED
#define _FGE_C_TEXTURE_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <FastEngine/texture_manager.hpp>

namespace fge
{

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
    Texture(const char* name);
    /**
     * \brief Copy a custom texture data pointer.
     *
     * \param data The custom texture data pointer
     */
    Texture( fge::texture::TextureDataPtr data );

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
    [[nodiscard]] sf::Vector2u getTextureSize() const;

    /**
     * \brief Get the texture data
     *
     * \return The texture data
     */
    [[nodiscard]] const fge::texture::TextureDataPtr& getData() const;
    /**
     * \brief Get the name of the texture
     *
     * \return The name of the texture
     */
    [[nodiscard]] const std::string& getName() const;

    /**
     * \brief Get the texture data by its name
     *
     * \param name The name of the loaded texture
     */
    fge::Texture& operator =( const std::string& name );
    fge::Texture& operator =( const char* name );
    /**
     * \brief Copy a custom texture data pointer.
     *
     * \param data The custom texture data pointer
     */
    fge::Texture& operator =( fge::texture::TextureDataPtr data );

    explicit operator sf::Texture*();
    explicit operator const sf::Texture*() const;

    /**
     * \brief Directly get the SFML texture
     *
     * \return The SFML texture
     */
    operator sf::Texture&();
    operator const sf::Texture&() const;

    operator std::string&();
    operator const std::string&() const;

private:
    fge::texture::TextureDataPtr g_data;
    std::string g_name;
};

}//end fge

#endif // _FGE_C_TEXTURE_HPP_INCLUDED
