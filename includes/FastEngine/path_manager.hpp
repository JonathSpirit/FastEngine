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

#ifndef _FGE_PATH_MANAGER_HPP_INCLUDED
#define _FGE_PATH_MANAGER_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <string>

namespace fge
{
namespace path
{

FGE_API const std::string& Get(const std::string& name);

FGE_API std::size_t GetPathSize();

FGE_API void Remove(const std::string& name);

FGE_API bool Check(const std::string& name);

FGE_API bool New(const std::string& name, const std::string& path);

FGE_API bool Replace(const std::string& name, const std::string& path);

}//end path
}//end fge


#endif // _FGE_PATH_MANAGER_HPP_INCLUDED
