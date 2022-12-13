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

#ifndef _FGE_C_PROPERTYLIST_HPP_INCLUDED
#define _FGE_C_PROPERTYLIST_HPP_INCLUDED

#include <FastEngine/C_property.hpp>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace fge
{

class PropertyList
{
public:
    using PropertyListType = std::unordered_map<std::string, fge::Property>;

    inline PropertyList() = default;
    inline PropertyList(const PropertyList& r) = default;
    inline PropertyList(PropertyList&& r) noexcept = default;
    inline ~PropertyList() = default;

    inline PropertyList& operator=(const PropertyList& r) = default;
    inline PropertyList& operator=(PropertyList&& r) noexcept = default;

    inline void delAllProperties();
    inline void delProperty(const std::string& key);

    inline bool checkProperty(const std::string& key) const;

    inline void setProperty(const std::string& key, const fge::Property& value);
    inline void setProperty(const std::string& key, fge::Property&& value);

    template<typename T>
    inline T* getPropertyType(const std::string& key);
    template<typename T>
    inline const T* getPropertyType(const std::string& key) const;

    inline fge::Property& getProperty(const std::string& key);
    inline const fge::Property& getProperty(const std::string& key) const;

    inline fge::Property& operator[](const std::string& key);
    inline const fge::Property& operator[](const std::string& key) const;

    inline std::size_t getPropertiesSize() const;

    inline fge::PropertyList::PropertyListType::iterator begin();
    inline fge::PropertyList::PropertyListType::iterator end();
    inline fge::PropertyList::PropertyListType::const_iterator begin() const;
    inline fge::PropertyList::PropertyListType::const_iterator end() const;

    inline fge::PropertyList::PropertyListType::const_iterator find(const std::string& key) const;
    inline fge::PropertyList::PropertyListType::iterator find(const std::string& key);

    inline void clearAllModificationFlags();
    inline std::size_t countAllModificationFlags() const;

private:
    fge::PropertyList::PropertyListType g_data;
};

} // namespace fge

#include <FastEngine/C_propertyList.inl>

#endif // _FGE_C_PROPERTYLIST_HPP_INCLUDED
