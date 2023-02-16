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

#ifndef _FGE_SCREEN_MANAGER_HPP_INCLUDED
#define _FGE_SCREEN_MANAGER_HPP_INCLUDED

#include "FastEngine/fastengine_extern.hpp"

#include <memory>
#include <string>

namespace fge
{
namespace screen
{

FGE_API void Uninit();

FGE_API void Close(const std::string& name);

FGE_API std::size_t GetScreenSize();

FGE_API std::shared_ptr<sf::RenderWindow> Get(const std::string& name);

FGE_API bool Check(const std::string& name);

FGE_API std::shared_ptr<sf::RenderWindow> New(const std::string& name);

} // namespace screen
} // namespace fge


#endif // _FGE_SCREEN_MANAGER_HPP_INCLUDED
