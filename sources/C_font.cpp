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
