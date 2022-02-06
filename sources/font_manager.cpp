#include "FastEngine/font_manager.hpp"

namespace fge
{
namespace font
{

namespace
{

fge::font::FontDataPtr _dataFontBad;
fge::font::FontDataType _dataFont;
std::mutex _dataMutex;

}//end

void FGE_API Init()
{
    if ( _dataFontBad == nullptr )
    {
        _dataFontBad = std::make_shared<fge::font::FontData>();
        _dataFontBad->_font = std::make_shared<sf::Font>();
        _dataFontBad->_valid = false;
    }
}
bool FGE_API IsInit()
{
    return _dataFontBad != nullptr;
}
void FGE_API Uninit()
{
    _dataFont.clear();
    _dataFontBad = nullptr;
}

std::size_t FGE_API GetFontSize()
{
    std::lock_guard<std::mutex> lck(_dataMutex);
    return _dataFont.size();
}

std::mutex& FGE_API GetMutex()
{
    return _dataMutex;
}

fge::font::FontDataType::const_iterator FGE_API GetCBegin()
{
    return _dataFont.cbegin();
}
fge::font::FontDataType::const_iterator FGE_API GetCEnd()
{
    return _dataFont.cend();
}

const fge::font::FontDataPtr& FGE_API GetBadFont()
{
    return _dataFontBad;
}
fge::font::FontDataPtr FGE_API GetFont(const std::string& name)
{
    if (name == FGE_FONT_BAD)
    {
        return _dataFontBad;
    }

    std::lock_guard<std::mutex> lck(_dataMutex);
    auto it = _dataFont.find(name);

    if (it != _dataFont.end())
    {
        return it->second;
    }
    return _dataFontBad;
}

bool FGE_API Check(const std::string& name)
{
    if (name == FGE_FONT_BAD)
    {
        return false;
    }

    std::lock_guard<std::mutex> lck(_dataMutex);
    auto it = _dataFont.find(name);

    if (it != _dataFont.end())
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

    std::lock_guard<std::mutex> lck(_dataMutex);
    auto it = _dataFont.find(name);

    if (it != _dataFont.end())
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

    _dataFont[name] = std::move(buff);
    return true;
}

bool FGE_API Unload(const std::string& name)
{
    if (name == FGE_FONT_BAD)
    {
        return false;
    }

    std::lock_guard<std::mutex> lck(_dataMutex);
    auto it = _dataFont.find(name);

    if (it != _dataFont.end())
    {
        it->second->_valid = false;
        it->second->_font = _dataFontBad->_font;
        _dataFont.erase(it);
        return true;
    }
    return false;
}
void FGE_API UnloadAll()
{
    std::lock_guard<std::mutex> lck(_dataMutex);

    for (auto & data : _dataFont)
    {
        data.second->_valid = false;
        data.second->_font = _dataFontBad->_font;
    }
    _dataFont.clear();
}

bool FGE_API Push(const std::string& name, const fge::font::FontDataPtr& data)
{
    if (name == FGE_FONT_BAD)
    {
        return false;
    }

    std::lock_guard<std::mutex> lck(_dataMutex);
    if ( fge::font::Check(name) )
    {
        return false;
    }

    _dataFont.emplace(name, data);
    return true;
}

}//end font
}//end fge
