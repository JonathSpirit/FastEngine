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

#ifndef _FGE_C_FONT_HPP_INCLUDED
#define _FGE_C_FONT_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include "FastEngine/manager/font_manager.hpp"
#include <json.hpp>

namespace fge
{

namespace net
{

class Packet;

} // namespace net

/**
 * \class Font
 * \ingroup graphics
 * \brief This class is a wrapper for the font manger to allow easy manipulation
 */
class FGE_API Font
{
public:
    Font();
    /**
     * \brief Get the font data by its name
     *
     * \param name The name of the loaded font
     */
    Font(std::string name);
    Font(const char* name);
    /**
     * \brief Copy a custom font data pointer.
     *
     * \param data The custom font data pointer
     */
    Font(fge::font::FontDataPtr data);

    /**
     * \brief Clear the font data
     *
     * This function clear the font data by setting it to the default font.
     */
    void clear();

    /**
     * \brief Check if the font is valid (not unloaded)
     *
     * \return True if the font is valid, false otherwise
     */
    [[nodiscard]] bool valid() const;

    /**
     * \brief Get the font data
     *
     * \return The font data
     */
    [[nodiscard]] const fge::font::FontDataPtr& getData() const;
    /**
     * \brief Get the name of the font
     *
     * \return The name of the font
     */
    [[nodiscard]] const std::string& getName() const;

    /**
     * \brief Get the font data by its name
     *
     * \param name The name of the loaded font
     */
    fge::Font& operator=(std::string name);
    fge::Font& operator=(const char* name);
    /**
     * \brief Copy a custom font data pointer.
     *
     * \param data The custom font data pointer
     */
    fge::Font& operator=(fge::font::FontDataPtr data);

    explicit operator sf::Font*();
    explicit operator const sf::Font*() const;

    /**
     * \brief Directly get the SFML font
     *
     * \return The SFML font
     */
    operator sf::Font&();
    operator const sf::Font&() const;

private:
    fge::font::FontDataPtr g_data;
    std::string g_name;
};

FGE_API const fge::net::Packet& operator>>(const fge::net::Packet& pck, fge::Font& data);
FGE_API fge::net::Packet& operator<<(fge::net::Packet& pck, const fge::Font& data);

FGE_API void to_json(nlohmann::json& j, const fge::Font& p);
FGE_API void from_json(const nlohmann::json& j, fge::Font& p);

} // namespace fge

#endif // _FGE_C_FONT_HPP_INCLUDED
