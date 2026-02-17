/*
 * Copyright 2026 Guillaume Guillet
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

namespace fge
{
namespace string
{

template<class T>
std::string ToStr(std::list<T> const& val, char separator)
{
    std::string result;
    for (auto it = val.cbegin(); it != val.cend(); ++it)
    {
        result += fge::string::ToStr(*it) + separator;
    }
    if (!result.empty())
    {
        result.pop_back();
    }
    return result;
}
template<class T>
std::string ToStr(std::vector<T> const& val, char separator)
{
    std::string result;
    for (auto it = val.cbegin(); it != val.cend(); ++it)
    {
        result += fge::string::ToStr(*it) + separator;
    }
    if (!result.empty())
    {
        result.pop_back();
    }
    return result;
}

template<class T>
std::string ToStr(std::optional<T> const& val)
{
    if (val.has_value())
    {
        return fge::string::ToStr(val.value());
    }
    return "NO_VALUE";
}

} // namespace string
} // namespace fge
