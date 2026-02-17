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

#ifndef _FGE_C_TAGLIST_HPP_INCLUDED
#define _FGE_C_TAGLIST_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include <set>
#include <string>

namespace fge
{

class FGE_API TagList
{
public:
    using TagListType = std::set<std::string, std::less<>>;

    TagList() = default;
    ~TagList() = default;

    void clear();

    void add(std::string_view tag);
    void del(std::string_view tag);

    [[nodiscard]] bool check(std::string_view tag) const;

    [[nodiscard]] std::size_t getSize() const;

    [[nodiscard]] fge::TagList::TagListType::const_iterator begin() const;
    [[nodiscard]] fge::TagList::TagListType::const_iterator end() const;

private:
    fge::TagList::TagListType g_tags;
};

} // namespace fge

#endif // _FGE_C_TAGLIST_HPP_INCLUDED
