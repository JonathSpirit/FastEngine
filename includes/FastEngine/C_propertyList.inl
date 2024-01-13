/*
 * Copyright 2024 Guillaume Guillet
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

bool PropertyList::checkProperty(std::string const& key) const
{
    return this->g_data.find(key) != this->g_data.cend();
}

void PropertyList::setProperty(std::string const& key, fge::Property const& value)
{
    this->g_data[key] = value;
}
void PropertyList::setProperty(std::string const& key, fge::Property&& value)
{
    this->g_data[key] = std::move(value);
}

template<typename T>
T* PropertyList::getPropertyType(std::string const& key)
{
    return this->g_data[key].getPtr<T>();
}
template<typename T>
T const* PropertyList::getPropertyType(std::string const& key) const
{
    auto it = this->g_data.find(key);
    if (it != this->g_data.cend())
    {
        return it->second.getPtr<T>();
    }
    return nullptr;
}

fge::Property& PropertyList::getProperty(std::string const& key)
{
    return this->g_data[key];
}
fge::Property const& PropertyList::getProperty(std::string const& key) const
{
    auto it = this->g_data.find(key);
    if (it != this->g_data.cend())
    {
        return it->second;
    }
    throw fge::Exception("key not found !");
}

fge::Property& PropertyList::operator[](std::string const& key)
{
    return this->g_data[key];
}
fge::Property const& PropertyList::operator[](std::string const& key) const
{
    auto it = this->g_data.find(key);
    if (it != this->g_data.cend())
    {
        return it->second;
    }
    throw fge::Exception("key not found !");
}

std::size_t PropertyList::getPropertiesSize() const
{
    return this->g_data.size();
}

fge::PropertyList::Type::iterator PropertyList::begin()
{
    return this->g_data.begin();
}
fge::PropertyList::Type::iterator PropertyList::end()
{
    return this->g_data.end();
}
fge::PropertyList::Type::const_iterator PropertyList::begin() const
{
    return this->g_data.begin();
}
fge::PropertyList::Type::const_iterator PropertyList::end() const
{
    return this->g_data.end();
}

fge::PropertyList::Type::const_iterator PropertyList::find(std::string const& key) const
{
    return this->g_data.find(key);
}
fge::PropertyList::Type::iterator PropertyList::find(std::string const& key)
{
    return this->g_data.find(key);
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
