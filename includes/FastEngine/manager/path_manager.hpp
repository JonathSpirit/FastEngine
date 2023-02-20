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

#ifndef _FGE_PATH_MANAGER_HPP_INCLUDED
#define _FGE_PATH_MANAGER_HPP_INCLUDED

#include "FastEngine/fastengine_extern.hpp"
#include <filesystem>
#include <string>

namespace fge::path
{

FGE_API std::filesystem::path Get(std::string_view name);

FGE_API std::size_t GetPathSize();

FGE_API void Remove(std::string_view name);

FGE_API bool Check(std::string_view name);

FGE_API bool New(std::string_view name, std::filesystem::path path);

FGE_API bool Replace(std::string_view name, std::filesystem::path path);

} // namespace fge::path


#endif // _FGE_PATH_MANAGER_HPP_INCLUDED
