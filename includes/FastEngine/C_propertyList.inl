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

void PropertyList::delAllProperties()
{
    this->g_data.clear();
}
void PropertyList::delProperty(std::string const& key)
{
    this->g_data.erase(key);
}

template<class T>
bool PropertyList::findProperty(std::string const& key) const
{
    auto it = this->g_data.find(key);
    if (it != this->g_data.cend())
    {
        return it->second.isType<T>();
    }
    return false;
}
bool PropertyList::findProperty(std::string const& key) const
{
    return this->g_data.find(key) != this->g_data.cend();
}

template<class T>
void PropertyList::setProperty(std::string const& key, T&& value)
{
    this->g_data[key] = std::forward<T>(value);
}

template<class T>
T* PropertyList::getProperty(std::string const& key)
{
    return this->g_data[key].getPtr<T>();
}
template<class T>
T const* PropertyList::getProperty(std::string const& key) const
{
    auto it = this->g_data.find(key);
    if (it != this->g_data.cend())
    {
        return it->second.getPtr<T>();
    }
    return nullptr;
}
template<class T, class TDefault>
T& PropertyList::getProperty(std::string const& key, TDefault&& defaultValue)
{
    auto& data = this->g_data[key];
    if (!data.isType<T>())
    {
        return data.setType<T>() = std::forward<TDefault>(defaultValue);
    }
    return data.get<T>();
}

fge::Property& PropertyList::getProperty(std::string const& key)
{
    return this->g_data[key];
}
fge::Property const* PropertyList::getProperty(std::string const& key) const
{
    auto it = this->g_data.find(key);
    if (it != this->g_data.cend())
    {
        return &it->second;
    }
    return nullptr;
}

fge::Property& PropertyList::operator[](std::string const& key)
{
    return this->g_data[key];
}
fge::Property const* PropertyList::operator[](std::string const& key) const
{
    auto it = this->g_data.find(key);
    if (it != this->g_data.cend())
    {
        return &it->second;
    }
    return nullptr;
}

std::size_t PropertyList::count() const
{
    return this->g_data.size();
}

fge::PropertyList::DataType::iterator PropertyList::begin()
{
    return this->g_data.begin();
}
fge::PropertyList::DataType::iterator PropertyList::end()
{
    return this->g_data.end();
}
fge::PropertyList::DataType::const_iterator PropertyList::begin() const
{
    return this->g_data.begin();
}
fge::PropertyList::DataType::const_iterator PropertyList::end() const
{
    return this->g_data.end();
}

void PropertyList::clearAllModificationFlags()
{
    for (auto& data: this->g_data)
    {
        data.second.setModifiedFlag(false);
    }
}
std::size_t PropertyList::countAllModificationFlags() const
{
    std::size_t counter{0};

    for (auto const& data: this->g_data)
    {
        if (data.second.isModified())
        {
            ++counter;
        }
    }

    return counter;
}

} // namespace fge
