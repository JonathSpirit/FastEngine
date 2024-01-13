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

#ifndef _FGE_C_PROPERTYLIST_HPP_INCLUDED
#define _FGE_C_PROPERTYLIST_HPP_INCLUDED

#include "FastEngine/C_property.hpp"
#include "FastEngine/fge_except.hpp"
#include <string>
#include <unordered_map>

namespace fge
{

class PropertyList
{
public:
    using Type = std::unordered_map<std::string, fge::Property>;

    inline PropertyList() = default;
    inline PropertyList(PropertyList const& r) = default;
    inline PropertyList(PropertyList&& r) noexcept = default;
    inline ~PropertyList() = default;

    inline PropertyList& operator=(PropertyList const& r) = default;
    inline PropertyList& operator=(PropertyList&& r) noexcept = default;

    inline void delAllProperties();
    inline void delProperty(std::string const& key);

    inline bool checkProperty(std::string const& key) const;

    inline void setProperty(std::string const& key, fge::Property const& value);
    inline void setProperty(std::string const& key, fge::Property&& value);

    template<typename T>
    inline T* getPropertyType(std::string const& key);
    template<typename T>
    inline T const* getPropertyType(std::string const& key) const;

    inline fge::Property& getProperty(std::string const& key);
    inline fge::Property const& getProperty(std::string const& key) const;

    inline fge::Property& operator[](std::string const& key);
    inline fge::Property const& operator[](std::string const& key) const;

    inline std::size_t getPropertiesSize() const;

    inline fge::PropertyList::Type::iterator begin();
    inline fge::PropertyList::Type::iterator end();
    inline fge::PropertyList::Type::const_iterator begin() const;
    inline fge::PropertyList::Type::const_iterator end() const;

    inline fge::PropertyList::Type::const_iterator find(std::string const& key) const;
    inline fge::PropertyList::Type::iterator find(std::string const& key);

    inline void clearAllModificationFlags();
    inline std::size_t countAllModificationFlags() const;

private:
    fge::PropertyList::Type g_data;
};

} // namespace fge

#include "FastEngine/C_propertyList.inl"

#endif // _FGE_C_PROPERTYLIST_HPP_INCLUDED
