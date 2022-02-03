#include "FastEngine/C_font.hpp"

namespace fge
{

FGE_API Font::Font() :
    g_data(fge::font::GetBadFont()),
    g_name(FGE_FONT_BAD)
{
}
FGE_API Font::Font( const std::string& name ) :
    g_data(fge::font::GetFont(name)),
    g_name(name)
{
}
FGE_API Font::Font( const char* name ) :
    g_data(fge::font::GetFont(std::string(name))),
    g_name(name)
{
}
FGE_API Font::Font( const fge::font::FontDataPtr& data ) :
    g_data(data),
    g_name(FGE_FONT_BAD)
{
}

void FGE_API Font::clear()
{
    this->g_data = fge::font::GetBadFont();
    this->g_name = FGE_FONT_BAD;
}

bool FGE_API Font::valid() const
{
    return this->g_data->_valid;
}

const fge::font::FontDataPtr& FGE_API Font::getData() const
{
    return this->g_data;
}
const std::string& FGE_API Font::getName() const
{
    return this->g_name;
}

void FGE_API Font::operator =( const std::string& name )
{
    this->g_name = name;
    this->g_data = fge::font::GetFont(name);
}
void FGE_API Font::operator =( const char* name )
{
    this->g_name = std::string(name);
    this->g_data = fge::font::GetFont(this->g_name);
}
void FGE_API Font::operator =( const fge::font::FontDataPtr& data )
{
    this->g_name = FGE_FONT_BAD;
    this->g_data = data;
}

FGE_API Font::operator sf::Font*()
{
    return this->g_data->_font.get();
}
FGE_API Font::operator const sf::Font*() const
{
    return this->g_data->_font.get();
}

FGE_API Font::operator sf::Font&()
{
    return *this->g_data->_font;
}
FGE_API Font::operator const sf::Font&() const
{
    return *this->g_data->_font;
}

FGE_API Font::operator std::string&()
{
    return this->g_name;
}
FGE_API Font::operator const std::string&() const
{
    return this->g_name;
}

}//end fge
