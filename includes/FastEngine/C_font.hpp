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

#ifndef _FGE_C_FONT_HPP_INCLUDED
#define _FGE_C_FONT_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "FastEngine/manager/font_manager.hpp"
#include "json.hpp"

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
    Font(char const* name);
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
    [[nodiscard]] fge::font::FontDataPtr const& getData() const;
    /**
     * \brief Get the name of the font
     *
     * \return The name of the font
     */
    [[nodiscard]] std::string const& getName() const;

    /**
     * \brief Get the font data by its name
     *
     * \param name The name of the loaded font
     */
    fge::Font& operator=(std::string name);
    fge::Font& operator=(char const* name);
    /**
     * \brief Copy a custom font data pointer.
     *
     * \param data The custom font data pointer
     */
    fge::Font& operator=(fge::font::FontDataPtr data);

    /**
     * \brief Retrieve the internal FreeType font pointer
     *
     * \warning Will never be \b nullptr if the font manager was correctly initialized.
     *
     * \return The FreeType font pointer
     */
    [[nodiscard]] fge::FreeTypeFont* retrieve();
    [[nodiscard]] fge::FreeTypeFont const* retrieve() const;

private:
    fge::font::FontDataPtr g_data;
    std::string g_name;
};

FGE_API fge::net::Packet const& operator>>(fge::net::Packet const& pck, fge::Font& data);
FGE_API fge::net::Packet& operator<<(fge::net::Packet& pck, fge::Font const& data);

FGE_API void to_json(nlohmann::json& j, fge::Font const& p);
FGE_API void from_json(nlohmann::json const& j, fge::Font& p);

} // namespace fge

#endif // _FGE_C_FONT_HPP_INCLUDED
