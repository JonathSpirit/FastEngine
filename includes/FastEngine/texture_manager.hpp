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

void FGE_API Init();
bool FGE_API IsInit();
void FGE_API Uninit();

std::size_t FGE_API GetTextureSize();

std::mutex& GetMutex();
fge::texture::TextureDataType::const_iterator FGE_API GetCBegin();
fge::texture::TextureDataType::const_iterator FGE_API GetCEnd();

const fge::texture::TextureDataPtr& FGE_API GetBadTexture();
fge::texture::TextureDataPtr FGE_API GetTexture(const std::string& name);

bool FGE_API Check(const std::string& name);

bool FGE_API LoadFromImage(const std::string& name, const sf::Image& image);
bool FGE_API LoadFromFile(const std::string& name, const std::string& path);
bool FGE_API Unload(const std::string& name);
void FGE_API UnloadAll();

bool FGE_API Push(const std::string& name, const fge::texture::TextureDataPtr& data);

}//end texture
}//end fge

#endif // _FGE_TEXTURE_MANAGER_HPP_INCLUDED
