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

#include "FastEngine/C_font.hpp"
#include "FastEngine/network/C_packet.hpp"

namespace fge
{

using namespace fge::font;

Font::Font() :
        g_data(gManager.getBadElement()),
        g_name(FGE_FONT_BAD)
{}
Font::Font(std::string name) :
        g_data(gManager.getElement(name)),
        g_name(std::move(name))
{}
Font::Font(char const* name) :
        g_data(gManager.getElement(name)),
        g_name(name)
{}
Font::Font(std::string_view name) :
        g_data(gManager.getElement(name)),
        g_name(name)
{}
Font::Font(SharedDataType data) :
        g_data(std::move(data)),
        g_name(FGE_FONT_BAD)
{}

void Font::clear()
{
    this->g_data = gManager.getBadElement();
    this->g_name = FGE_FONT_BAD;
}

bool Font::valid() const
{
    return this->g_data->_valid;
}

Font::SharedDataType const& Font::getData() const
{
    return this->g_data;
}
std::string const& Font::getName() const
{
    return this->g_name;
}

fge::Font& Font::operator=(std::string name)
{
    this->g_name = std::move(name);
    this->g_data = gManager.getElement(this->g_name);
    return *this;
}
fge::Font& Font::operator=(char const* name)
{
    this->g_name = name;
    this->g_data = gManager.getElement(this->g_name);
    return *this;
}
fge::Font& Font::operator=(SharedDataType data)
{
    this->g_name = FGE_FONT_BAD;
    this->g_data = std::move(data);
    return *this;
}

fge::FreeTypeFont* Font::retrieve()
{
    return this->g_data->_ptr.get();
}
fge::FreeTypeFont const* Font::retrieve() const
{
    return this->g_data->_ptr.get();
}

fge::net::Packet const& operator>>(fge::net::Packet const& pck, fge::Font& data)
{
    std::string fontName;
    pck >> fontName;
    data = std::move(fontName);
    return pck;
}
fge::net::Packet& operator<<(fge::net::Packet& pck, fge::Font const& data)
{
    return pck << data.getName();
}

void to_json(nlohmann::json& j, fge::Font const& p)
{
    j = p.getName();
}
void from_json(nlohmann::json const& j, fge::Font& p)
{
    std::string fontName;
    j.get_to(fontName);
    p = std::move(fontName);
}

} // namespace fge
