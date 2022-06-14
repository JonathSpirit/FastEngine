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

namespace fge
{

Font::Font() :
    g_data(fge::font::GetBadFont()),
    g_name(FGE_FONT_BAD)
{
}
Font::Font( const std::string& name ) :
    g_data(fge::font::GetFont(name)),
    g_name(name)
{
}
Font::Font( const char* name ) :
    g_data(fge::font::GetFont(std::string(name))),
    g_name(name)
{
}
Font::Font( const fge::font::FontDataPtr& data ) :
    g_data(data),
    g_name(FGE_FONT_BAD)
{
}

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

fge::Font& Font::operator =( const std::string& name )
{
    this->g_name = name;
    this->g_data = fge::font::GetFont(name);
    return *this;
}
fge::Font& Font::operator =( const char* name )
{
    this->g_name = std::string(name);
    this->g_data = fge::font::GetFont(this->g_name);
    return *this;
}
fge::Font& Font::operator =( const fge::font::FontDataPtr& data )
{
    this->g_name = FGE_FONT_BAD;
    this->g_data = data;
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

Font::operator std::string&()
{
    return this->g_name;
}
Font::operator const std::string&() const
{
    return this->g_name;
}

}//end fge
