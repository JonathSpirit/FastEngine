/*
 * Copyright 2022 Guillaume Guillet
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "FastEngine/manager/font_manager.hpp"
#include "private/string_hash.hpp"

namespace fge::font
{

namespace
{

fge::font::FontDataPtr _dataFontBad;
std::unordered_map<std::string, fge::font::FontDataPtr, fge::priv::string_hash, std::equal_to<>> _dataFont;
std::mutex _dataMutex;

}//end

void Init()
{
    if ( _dataFontBad == nullptr )
    {
        _dataFontBad = std::make_shared<fge::font::FontData>();
        _dataFontBad->_font = std::make_shared<sf::Font>();
        _dataFontBad->_valid = false;
    }
}
bool IsInit()
{
    return _dataFontBad != nullptr;
}
void Uninit()
{
    _dataFont.clear();
    _dataFontBad = nullptr;
}

std::size_t GetFontSize()
{
    std::lock_guard<std::mutex> lck(_dataMutex);
    return _dataFont.size();
}

std::mutex& GetMutex()
{
    return _dataMutex;
}

fge::font::FontDataType::const_iterator GetCBegin()
{
    return _dataFont.cbegin();
}
fge::font::FontDataType::const_iterator GetCEnd()
{
    return _dataFont.cend();
}

const fge::font::FontDataPtr& GetBadFont()
{
    return _dataFontBad;
}
fge::font::FontDataPtr GetFont(std::string_view name)
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

bool Check(std::string_view name)
{
    if (name == FGE_FONT_BAD)
    {
        return false;
    }

    std::lock_guard<std::mutex> lck(_dataMutex);
    auto it = _dataFont.find(name);

    return it != _dataFont.end();
}

bool LoadFromFile(std::string_view name, std::filesystem::path path)
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

    auto tmpFont = std::make_shared<sf::Font>();

    if ( !tmpFont->loadFromFile(path.string()) )
    {
        return false;
    }

    fge::font::FontDataPtr buff = std::make_shared<fge::font::FontData>();
    buff->_font = std::move(tmpFont);
    buff->_valid = true;
    buff->_path = std::move(path);

    _dataFont[std::string{name}] = std::move(buff);
    return true;
}

bool Unload(std::string_view name)
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
void UnloadAll()
{
    std::lock_guard<std::mutex> lck(_dataMutex);

    for (auto & data : _dataFont)
    {
        data.second->_valid = false;
        data.second->_font = _dataFontBad->_font;
    }
    _dataFont.clear();
}

bool Push(std::string_view name, const fge::font::FontDataPtr& data)
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

}//end fge::font
