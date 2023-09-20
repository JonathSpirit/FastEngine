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

#include "FastEngine/manager/log_manager.hpp"

#include <cstdio>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace fge
{
namespace log
{

namespace
{

std::string _defaultFolder;

} // namespace

std::string const& SetDefaultFolder(std::string const& default_folder)
{
    _defaultFolder = default_folder;

    if (_defaultFolder.back() == '\\')
    {
        _defaultFolder.pop_back();
    }
    if ((_defaultFolder.length() != 0) && (_defaultFolder.back() != '/'))
    {
        _defaultFolder.push_back('/');
    }

    return _defaultFolder;
}

bool Remove(std::string const& name)
{
    int return_value;

    return_value = remove((_defaultFolder + name).c_str());
    if (!return_value)
    {
        return true;
    }
    return false;
}
bool Clean(std::string const& name)
{
    std::ofstream file_log;
    file_log.open((_defaultFolder + name).c_str());

    if (file_log)
    {
        file_log.close();
        return true;
    }
    return false;
}
bool Rename(std::string const& name, std::string const& new_name)
{
    int return_value;

    return_value = rename((_defaultFolder + name).c_str(), (_defaultFolder + new_name).c_str());
    if (!return_value)
    {
        return true;
    }
    return false;
}
bool Write(std::string const& name, std::string const& text, std::string const& desc)
{
    time_t t = time(nullptr); // get time now
    struct tm* now = localtime(&t);

    std::ofstream file_log;
    file_log.open(_defaultFolder + name, std::ios::app);

    if (!file_log)
    {
        return false;
    }

    file_log << "[" << std::setw(2) << std::setfill('0') << now->tm_mday << "." << std::setw(2) << std::setfill('0')
             << now->tm_mon + 1 << "." << 1900 + now->tm_year << "]";
    file_log << "(" << std::setw(2) << std::setfill('0') << now->tm_hour << ":" << std::setw(2) << std::setfill('0')
             << now->tm_min << ":" << std::setw(2) << std::setfill('0') << now->tm_sec << ") ";
    if (!desc.empty())
    {
        file_log << "[" << desc << "] ";
    }
    file_log << text << std::endl;

    return true;
}

} // namespace log
} // namespace fge
