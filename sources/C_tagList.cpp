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

#include "FastEngine/C_tagList.hpp"

namespace fge
{

void TagList::clear()
{
    this->g_tags.clear();
}

void TagList::add(std::string_view tag)
{
    this->g_tags.insert(std::move(std::string{tag}));
}
void TagList::del(std::string_view tag)
{
    auto it = this->g_tags.find(tag);
    if (it != this->g_tags.end())
    {
        this->g_tags.erase(it);
    }
}

bool TagList::check(std::string_view tag) const
{
    return this->g_tags.find(tag) != this->g_tags.end();
}

std::size_t TagList::getSize() const
{
    return this->g_tags.size();
}

fge::TagList::TagListType::const_iterator TagList::begin() const
{
    return this->g_tags.begin();
}
fge::TagList::TagListType::const_iterator TagList::end() const
{
    return this->g_tags.end();
}

}//end fge
