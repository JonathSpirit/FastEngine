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

#include "FastEngine/manager/path_manager.hpp"
#include "private/string_hash.hpp"
#include <unordered_map>
#include <mutex>

namespace fge::path
{

namespace
{

std::unordered_map<std::string, std::filesystem::path, fge::priv::string_hash, std::equal_to<>> _dataPath;
const std::filesystem::path _dataPathBad;
std::mutex _dataMutex;

}//end

std::filesystem::path Get(std::string_view name)
{
    std::scoped_lock<std::mutex> lock(_dataMutex);
    auto it = _dataPath.find(name);
    return (it != _dataPath.end()) ? it->second : _dataPathBad;
}

std::size_t GetPathSize()
{
    std::scoped_lock<std::mutex> lock(_dataMutex);
    return _dataPath.size();
}

void Remove(std::string_view name)
{
    std::scoped_lock<std::mutex> lock(_dataMutex);
    auto it = _dataPath.find(name);
    if (it != _dataPath.end())
    {
        _dataPath.erase(it);
    }
}

bool Check(std::string_view name)
{
    std::scoped_lock<std::mutex> lock(_dataMutex);
    return _dataPath.find(name) != _dataPath.cend();
}

bool New(std::string_view name, std::filesystem::path path)
{
    if ( fge::path::Check(name) )
    {
        return false;
    }

    std::scoped_lock<std::mutex> lock(_dataMutex);
    _dataPath[std::string{name}] = std::move(path);
    return true;
}

bool Replace(std::string_view name, std::filesystem::path path)
{
    std::scoped_lock<std::mutex> lock(_dataMutex);
    auto it = _dataPath.find(name);

    if ( it != _dataPath.end() )
    {
        it->second = std::move(path);
        return true;
    }
    return false;
}

}//end fge::path
