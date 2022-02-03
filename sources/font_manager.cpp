#include "FastEngine/font_manager.hpp"

namespace fge
{
namespace font
{

namespace
{

fge::font::FontDataPtr __dataFontBad;
fge::font::FontDataType __dataFont;
std::mutex __dataMutex;

}//end

void FGE_API Init()
{
    if ( __dataFontBad == nullptr )
    {
        __dataFontBad = std::make_shared<fge::font::FontData>();
        __dataFontBad->_font = std::make_shared<sf::Font>();
        __dataFontBad->_valid = false;
    }
}
bool FGE_API IsInit()
{
    return __dataFontBad != nullptr;
}
void FGE_API Uninit()
{
    __dataFont.clear();
    __dataFontBad = nullptr;
}

std::size_t FGE_API GetFontSize()
{
    std::lock_guard<std::mutex> lck(__dataMutex);
    return __dataFont.size();
}

std::mutex& FGE_API GetMutex()
{
    return __dataMutex;
}

fge::font::FontDataType::const_iterator FGE_API GetCBegin()
{
    return __dataFont.cbegin();
}
fge::font::FontDataType::const_iterator FGE_API GetCEnd()
{
    return __dataFont.cend();
}

const fge::font::FontDataPtr& FGE_API GetBadFont()
{
    return __dataFontBad;
}
fge::font::FontDataPtr FGE_API GetFont(const std::string& name)
{
    if (name == FGE_FONT_BAD)
    {
        return __dataFontBad;
    }

    std::lock_guard<std::mutex> lck(__dataMutex);
    fge::font::FontDataType::iterator it = __dataFont.find(name);

    if (it != __dataFont.end())
    {
        return it->second;
    }
    return __dataFontBad;
}

bool FGE_API Check(const std::string& name)
{
    if (name == FGE_FONT_BAD)
    {
        return false;
    }

    std::lock_guard<std::mutex> lck(__dataMutex);
    fge::font::FontDataType::iterator it = __dataFont.find(name);

    if (it != __dataFont.end())
    {
        return true;
    }
    return false;
}

bool FGE_API LoadFromFile(const std::string& name, const std::string& path)
{
    if (name == FGE_FONT_BAD)
    {
        return false;
    }

    std::lock_guard<std::mutex> lck(__dataMutex);
    fge::font::FontDataType::iterator it = __dataFont.find(name);

    if (it != __dataFont.end())
    {
        return false;
    }

    sf::Font* tmpFont = new sf::Font();

    if ( !tmpFont->loadFromFile(path) )
    {
        delete tmpFont;
        return false;
    }

    fge::font::FontDataPtr buff = std::make_shared<fge::font::FontData>();
    buff->_font = std::move( std::shared_ptr<sf::Font>(tmpFont) );
    buff->_valid = true;
    buff->_path = path;

    __dataFont[name] = std::move(buff);
    return true;
}

bool FGE_API Unload(const std::string& name)
{
    if (name == FGE_FONT_BAD)
    {
        return false;
    }

    std::lock_guard<std::mutex> lck(__dataMutex);
    fge::font::FontDataType::iterator it = __dataFont.find(name);

    if (it != __dataFont.end())
    {
        it->second->_valid = false;
        it->second->_font = __dataFontBad->_font;
        __dataFont.erase(it);
        return true;
    }
    return false;
}
void FGE_API UnloadAll()
{
    std::lock_guard<std::mutex> lck(__dataMutex);

    for (fge::font::FontDataType::iterator it=__dataFont.begin(); it!=__dataFont.end(); ++it)
    {
        it->second->_valid = false;
        it->second->_font = __dataFontBad->_font;
    }
    __dataFont.clear();
}

bool FGE_API Push(const std::string& name, const fge::font::FontDataPtr& data)
{
    if (name == FGE_FONT_BAD)
    {
        return false;
    }

    std::lock_guard<std::mutex> lck(__dataMutex);
    if ( fge::font::Check(name) )
    {
        return false;
    }

    __dataFont.emplace(name, data);
    return true;
}

}//end font
}//end fge
