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

#ifndef _FGE_C_PROPERTYLIST_HPP_INCLUDED
#define _FGE_C_PROPERTYLIST_HPP_INCLUDED

#include "FastEngine/C_property.hpp"
#include <string>
#include <unordered_map>

namespace fge
{

/**
 * \class PropertyList
 * \ingroup utility
 * \brief A class that map a string to a Property
 *
 * \see Property
 */
class PropertyList
{
public:
    using DataType = std::unordered_map<std::string, fge::Property>;

    inline PropertyList() = default;
    inline PropertyList(PropertyList const& r) = default;
    inline PropertyList(PropertyList&& r) noexcept = default;
    inline ~PropertyList() = default;

    inline PropertyList& operator=(PropertyList const& r) = default;
    inline PropertyList& operator=(PropertyList&& r) noexcept = default;

    inline void delAllProperties();
    inline void delProperty(std::string const& key);

    template<class T>
    [[nodiscard]] inline bool findProperty(std::string const& key) const;
    [[nodiscard]] inline bool findProperty(std::string const& key) const;

    template<class T>
    inline void setProperty(std::string const& key, T&& value);

    template<class T>
    [[nodiscard]] inline T* getProperty(std::string const& key);
    template<class T>
    [[nodiscard]] inline T const* getProperty(std::string const& key) const;
    template<class T, class TDefault>
    [[nodiscard]] inline T& getProperty(std::string const& key, TDefault&& defaultValue);

    [[nodiscard]] inline fge::Property& getProperty(std::string const& key);
    [[nodiscard]] inline fge::Property const* getProperty(std::string const& key) const;

    [[nodiscard]] inline fge::Property& operator[](std::string const& key);
    [[nodiscard]] inline fge::Property const* operator[](std::string const& key) const;

    [[nodiscard]] inline std::size_t count() const;

    [[nodiscard]] inline DataType::iterator begin();
    [[nodiscard]] inline DataType::iterator end();
    [[nodiscard]] inline DataType::const_iterator begin() const;
    [[nodiscard]] inline DataType::const_iterator end() const;

    inline void clearAllModificationFlags();
    [[nodiscard]] inline std::size_t countAllModificationFlags() const;

private:
    DataType g_data;
};

} // namespace fge

#include "FastEngine/C_propertyList.inl"

#endif // _FGE_C_PROPERTYLIST_HPP_INCLUDED
