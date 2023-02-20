/*
 * Copyright 2023 Guillaume Guillet
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

#include "FastEngine/manager/screen_manager.hpp"
#include <unordered_map>

namespace
{
std::unordered_map<std::string, std::shared_ptr<sf::RenderWindow>> __dataScreen;
} // namespace

namespace fge
{
namespace screen
{

void Uninit()
{
    for (auto it = __dataScreen.begin(); it != __dataScreen.end(); ++it)
    {
        it->second->close();
    }
    __dataScreen.clear();
}

void Close(const std::string& name)
{
    auto it = __dataScreen.find(name);
    if (it != __dataScreen.end())
    {
        it->second->close();
        __dataScreen.erase(it);
    }
}

std::size_t GetScreenSize()
{
    return __dataScreen.size();
}

std::shared_ptr<sf::RenderWindow> Get(const std::string& name)
{
    auto it = __dataScreen.find(name);
    return (it != __dataScreen.cend()) ? it->second : nullptr;
}

bool Check(const std::string& name)
{
    return __dataScreen.find(name) != __dataScreen.cend();
}

std::shared_ptr<sf::RenderWindow> New(const std::string& name)
{
    if (fge::screen::Check(name))
    {
        return nullptr;
    }

    std::shared_ptr<sf::RenderWindow> buffScreen = std::shared_ptr<sf::RenderWindow>(new sf::RenderWindow());

    __dataScreen[name] = buffScreen;

    return buffScreen;
}

} // namespace screen
} // namespace fge
