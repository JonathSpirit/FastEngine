#ifndef _FGE_C_FONT_HPP_INCLUDED
#define _FGE_C_FONT_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <FastEngine/font_manager.hpp>

namespace fge
{

class FGE_API Font
{
public:
    Font();
    Font( const std::string& name );
    Font( const char* name );
    Font( const fge::font::FontDataPtr& data );

    void clear();

    bool valid() const;

    const fge::font::FontDataPtr& getData() const;
    const std::string& getName() const;

    void operator =( const std::string& name );
    void operator =( const char* name );
    void operator =( const fge::font::FontDataPtr& data );

    explicit operator sf::Font*();
    explicit operator const sf::Font*() const;

    operator sf::Font&();
    operator const sf::Font&() const;

    operator std::string&();
    operator const std::string&() const;

private:
    fge::font::FontDataPtr g_data;
    std::string g_name;
};

}//end fge

#endif // _FGE_C_FONT_HPP_INCLUDED
