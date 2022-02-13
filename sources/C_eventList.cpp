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

    if ( it != this->g_events.end() )
    {
        return &it->second;
    }
    return nullptr;
}

}//end fge
