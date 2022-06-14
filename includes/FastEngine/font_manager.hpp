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
