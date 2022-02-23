#ifndef _FGE_C_TEXTURE_HPP_INCLUDED
#define _FGE_C_TEXTURE_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <FastEngine/texture_manager.hpp>

namespace fge
{

class FGE_API Texture
{
public:
    Texture();
    Texture( const std::string& name );
    Texture( const char* name );
    Texture( fge::texture::TextureDataPtr data );

    void clear();
    void refresh();

    [[nodiscard]] bool valid() const;

    [[nodiscard]] sf::Vector2u getTextureSize() const;

    [[nodiscard]] const fge::texture::TextureDataPtr& getData() const;
    [[nodiscard]] const std::string& getName() const;

    fge::Texture& operator =( const std::string& name );
    fge::Texture& operator =( const char* name );
    fge::Texture& operator =( fge::texture::TextureDataPtr data );

    explicit operator sf::Texture*();
    explicit operator const sf::Texture*() const;

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
