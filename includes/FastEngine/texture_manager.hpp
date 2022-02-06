#ifndef _FGE_TEXTURE_MANAGER_HPP_INCLUDED
#define _FGE_TEXTURE_MANAGER_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>

#include <SFML/Graphics/Texture.hpp>
#include <string>
#include <unordered_map>
#include <mutex>
#include <memory>

#define FGE_TEXTURE_DEFAULT FGE_TEXTURE_BAD
#define FGE_TEXTURE_BAD ""

namespace fge
{
namespace texture
{

struct TextureData
{
    std::shared_ptr<sf::Texture> _texture;
    bool _valid;
    std::string _path;
};

using TextureDataPtr = std::shared_ptr<fge::texture::TextureData>;
using TextureDataType = std::unordered_map<std::string, fge::texture::TextureDataPtr>;

FGE_API void Init();
FGE_API bool IsInit();
FGE_API void Uninit();

FGE_API std::size_t GetTextureSize();

FGE_API std::mutex& GetMutex();
FGE_API fge::texture::TextureDataType::const_iterator  GetCBegin();
FGE_API fge::texture::TextureDataType::const_iterator GetCEnd();

FGE_API const fge::texture::TextureDataPtr& GetBadTexture();
FGE_API fge::texture::TextureDataPtr GetTexture(const std::string& name);

FGE_API bool Check(const std::string& name);

FGE_API bool LoadFromImage(const std::string& name, const sf::Image& image);
FGE_API bool LoadFromFile(const std::string& name, const std::string& path);
FGE_API bool Unload(const std::string& name);
FGE_API void UnloadAll();

FGE_API bool Push(const std::string& name, const fge::texture::TextureDataPtr& data);

}//end texture
}//end fge

#endif // _FGE_TEXTURE_MANAGER_HPP_INCLUDED
