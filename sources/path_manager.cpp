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

#include "FastEngine/path_manager.hpp"
#include <unordered_map>

namespace fge
{
namespace path
{

namespace
{

std::unordered_map<std::string, std::string> _dataPath;
const std::string _dataPathBad;

}//end

const std::string& Get(const std::string& name)
{
    auto it = _dataPath.find(name);
    return (it != _dataPath.cend()) ? it->second : _dataPathBad;
}

std::size_t GetPathSize()
{
    return _dataPath.size();
}

void Remove(const std::string& name)
{
    _dataPath.erase(name);
}

bool Check(const std::string& name)
{
    return _dataPath.find(name) != _dataPath.cend();
}

bool New(const std::string& name, const std::string& path)
{
    if ( fge::path::Check(name) )
    {
        return false;
    }

    _dataPath[name] = path;
    return true;
}

bool Replace(const std::string& name, const std::string& path)
{
    auto it = _dataPath.find(name);

    if ( it != _dataPath.end() )
    {
        it->second = path;
        return true;
    }
    return false;
}

}//end path
}//end fge
