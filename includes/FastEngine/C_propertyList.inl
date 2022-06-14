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

namespace fge
{

PropertyList::PropertyList(const PropertyList& r) :
        g_data(r.g_data)
{
}
PropertyList::PropertyList(PropertyList&& r) noexcept :
        g_data(std::move(r.g_data))
{
}

void PropertyList::delAllProperties()
{
    this->g_data.clear();
}
void PropertyList::delProperty(const std::string& key)
{
    this->g_data.erase(key);
}

bool PropertyList::checkProperty(const std::string& key) const
{
    return this->g_data.find(key) != this->g_data.cend();
}

void PropertyList::setProperty(const std::string& key, const fge::Property& value)
{
    this->g_data[key] = value;
}
void PropertyList::setProperty(const std::string& key, fge::Property&& value)
{
    this->g_data[key] = std::move(value);
}

template <typename T>
T* PropertyList::getPropertyType(const std::string& key)
{
    return this->g_data[key].getPtr<T>();
}
template <typename T>
const T* PropertyList::getPropertyType(const std::string& key) const
{
    auto it = this->g_data.find(key);
    if (it != this->g_data.cend())
    {
        return it->second.getPtr<T>();
    }
    return nullptr;
}

fge::Property& PropertyList::getProperty(const std::string& key)
{
    return this->g_data[key];
}
const fge::Property& PropertyList::getProperty(const std::string& key) const
{
    auto it = this->g_data.find(key);
    if (it != this->g_data.cend())
    {
        return it->second;
    }
    throw std::logic_error("key not found !");
}

fge::Property& PropertyList::operator[] (const std::string& key)
{
    return this->g_data[key];
}
const fge::Property& PropertyList::operator[] (const std::string& key) const
{
    auto it = this->g_data.find(key);
    if (it != this->g_data.cend())
    {
        return it->second;
    }
    throw std::logic_error("key not found !");
}

std::size_t PropertyList::getPropertiesSize() const
{
    return this->g_data.size();
}

fge::PropertyList::PropertyListType::iterator PropertyList::begin()
{
    return this->g_data.begin();
}
fge::PropertyList::PropertyListType::iterator PropertyList::end()
{
    return this->g_data.end();
}
fge::PropertyList::PropertyListType::const_iterator PropertyList::cbegin()
{
    return this->g_data.cbegin();
}
fge::PropertyList::PropertyListType::const_iterator PropertyList::cend()
{
    return this->g_data.cend();
}

fge::PropertyList::PropertyListType::const_iterator PropertyList::find(const std::string& key) const
{
    return this->g_data.find(key);
}
fge::PropertyList::PropertyListType::iterator PropertyList::find(const std::string& key)
{
    return this->g_data.find(key);
}

void PropertyList::clearAllModificationFlags()
{
    for (auto& data : this->g_data)
    {
        data.second.setModifiedFlag(false);
    }
}
std::size_t PropertyList::countAllModificationFlags() const
{
    std::size_t counter{0};

    for (const auto& data : this->g_data)
    {
        if ( data.second.isModified() )
        {
            ++counter;
        }
    }

    return counter;
}

}//end fge
