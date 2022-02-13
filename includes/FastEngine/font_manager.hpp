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

FGE_API void Init();
FGE_API bool IsInit();
FGE_API void Uninit();

FGE_API std::size_t GetFontSize();

FGE_API std::mutex& GetMutex();
FGE_API fge::font::FontDataType::const_iterator GetCBegin();
FGE_API fge::font::FontDataType::const_iterator GetCEnd();

FGE_API const fge::font::FontDataPtr& GetBadFont();
FGE_API fge::font::FontDataPtr GetFont(const std::string& name);

FGE_API bool Check(const std::string& name);

FGE_API bool LoadFromFile(const std::string& name, const std::string& path);
FGE_API bool Unload(const std::string& name);
FGE_API void UnloadAll();

FGE_API bool Push(const std::string& name, const fge::font::FontDataPtr& data);

}//end font
}//end fge

#endif // _FGE_FONT_MANAGER_HPP_INCLUDED
