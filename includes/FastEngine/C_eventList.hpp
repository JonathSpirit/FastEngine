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

#ifndef _FGE_C_EVENTLIST_HPP_INCLUDED
#define _FGE_C_EVENTLIST_HPP_INCLUDED

#include "FastEngine/fastengine_extern.hpp"
#include "FastEngine/C_event.hpp"
#include <unordered_map>

namespace fge
{

/**
 * \class EventList
 * \ingroup objectControl
 * \brief This class is used to manage events
 *
 * You can associate a string to an event class for easy identification.
 *
 * \see Event
 */
class FGE_API EventList
{
public:
    EventList() = default;
    ~EventList() = default;

    /**
     * \brief Add an event to the list
     *
     * \param name The name of the event
     * \return \b true if the event was added, \b false otherwise
     */
    bool add(const std::string& name);
    /**
     * \brief Remove an event from the list
     *
     * \param name The name of the event
     * \return \b true if the event was removed, \b false otherwise
     */
    bool del(const std::string& name);
    /**
     * \brief Remove all events from the list
     */
    void delAll();

    /**
     * \brief Get an event from the list
     *
     * \param name The name of the event
     * \return An pointer to the event, or \b nullptr if the event doesn't exist
     */
    fge::Event* get(const std::string& name);

private:
    std::unordered_map<std::string, fge::Event> g_events;
};

} // namespace fge

#endif // _FGE_C_EVENTLIST_HPP_INCLUDED
