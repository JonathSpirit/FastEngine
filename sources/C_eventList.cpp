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

#include "FastEngine/C_eventList.hpp"

namespace fge
{

bool EventList::add(const std::string& name)
{
    return this->g_events.emplace(name, fge::Event()).second;
}
bool EventList::del(const std::string& name)
{
    return this->g_events.erase(name) >= 1;
}
void EventList::delAll()
{
    this->g_events.clear();
}

fge::Event* EventList::get(const std::string& name)
{
    auto it = this->g_events.find(name);

    if (it != this->g_events.end())
    {
        return &it->second;
    }
    return nullptr;
}

} // namespace fge
