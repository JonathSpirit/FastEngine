#ifndef _FGE_FONT_MANAGER_HPP_INCLUDED
#define _FGE_FONT_MANAGER_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>

#include <SFML/Graphics/Font.hpp>
#include <string>
#include <unordered_map>
#include <mutex>
#include <memory>

#define FGE_FONT_DEFAULT FGE_FONT_BAD
#define FGE_FONT_BAD ""

namespace fge
{
namespace font
{

struct FontData
{
    std::shared_ptr<sf::Font> _font;
    bool _valid;
    std::string _path;
};

using FontDataPtr = std::shared_ptr<fge::font::FontData>;
using FontDataType = std::unordered_map<std::string, fge::font::FontDataPtr>;

void FGE_API Init();
bool FGE_API IsInit();
void FGE_API Uninit();

std::size_t FGE_API GetFontSize();

std::mutex& FGE_API GetMutex();
fge::font::FontDataType::const_iterator FGE_API GetCBegin();
fge::font::FontDataType::const_iterator FGE_API GetCEnd();

const fge::font::FontDataPtr& FGE_API GetBadFont();
fge::font::FontDataPtr FGE_API GetFont(const std::string& name);

bool FGE_API Check(const std::string& name);

bool FGE_API LoadFromFile(const std::string& name, const std::string& path);
bool FGE_API Unload(const std::string& name);
void FGE_API UnloadAll();

bool FGE_API Push(const std::string& name, const fge::font::FontDataPtr& data);

}//end font
}//end fge

#endif // _FGE_FONT_MANAGER_HPP_INCLUDED
