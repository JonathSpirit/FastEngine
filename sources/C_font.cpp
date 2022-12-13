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

#include "FastEngine/C_font.hpp"
#include "FastEngine/C_packet.hpp"

namespace fge
{

Font::Font() :
        g_data(fge::font::GetBadFont()),
        g_name(FGE_FONT_BAD)
{}
Font::Font(std::string name) :
        g_data(fge::font::GetFont(name)),
        g_name(std::move(name))
{}
Font::Font(const char* name) :
        g_data(fge::font::GetFont(name)),
        g_name(name)
{}
Font::Font(fge::font::FontDataPtr data) :
        g_data(std::move(data)),
        g_name(FGE_FONT_BAD)
{}

void Font::clear()
{
    this->g_data = fge::font::GetBadFont();
    this->g_name = FGE_FONT_BAD;
}

bool Font::valid() const
{
    return this->g_data->_valid;
}

const fge::font::FontDataPtr& Font::getData() const
{
    return this->g_data;
}
const std::string& Font::getName() const
{
    return this->g_name;
}

fge::Font& Font::operator=(std::string name)
{
    this->g_name = std::move(name);
    this->g_data = fge::font::GetFont(this->g_name);
    return *this;
}
fge::Font& Font::operator=(const char* name)
{
    this->g_name = name;
    this->g_data = fge::font::GetFont(this->g_name);
    return *this;
}
fge::Font& Font::operator=(fge::font::FontDataPtr data)
{
    this->g_name = FGE_FONT_BAD;
    this->g_data = std::move(data);
    return *this;
}

Font::operator sf::Font*()
{
    return this->g_data->_font.get();
}
Font::operator const sf::Font*() const
{
    return this->g_data->_font.get();
}

Font::operator sf::Font&()
{
    return *this->g_data->_font;
}
Font::operator const sf::Font&() const
{
    return *this->g_data->_font;
}

const fge::net::Packet& operator>>(const fge::net::Packet& pck, fge::Font& data)
{
    std::string fontName;
    pck >> fontName;
    data = std::move(fontName);
    return pck;
}
fge::net::Packet& operator<<(fge::net::Packet& pck, const fge::Font& data)
{
    return pck << data.getName();
}

void to_json(nlohmann::json& j, const fge::Font& p)
{
    j = p.getName();
}
void from_json(const nlohmann::json& j, fge::Font& p)
{
    std::string fontName;
    j.get_to(fontName);
    p = std::move(fontName);
}

} // namespace fge
