/*
 * Copyright 2025 Guillaume Guillet
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

#ifndef _FGE_LOG_MANAGER_HPP_INCLUDED
#define _FGE_LOG_MANAGER_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"

#include <string>

namespace fge
{
namespace log
{

FGE_API std::string const& SetDefaultFolder(std::string const& default_folder);

FGE_API bool Remove(std::string const& name);
FGE_API bool Clean(std::string const& name);
FGE_API bool Rename(std::string const& name, std::string const& new_name);
FGE_API bool Write(std::string const& name, std::string const& text, std::string const& desc = std::string());

} // namespace log
} // namespace fge

#endif // _FGE_LOG_MANAGER_HPP_INCLUDED
