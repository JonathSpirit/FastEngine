#include "FastEngine/C_eventList.hpp"

namespace fge
{

bool FGE_API EventList::add(const std::string& name)
{
    return this->g_events.emplace(name, fge::Event()).second;
}
bool FGE_API EventList::del(const std::string& name)
{
    return this->g_events.erase(name) >= 1;
}
void FGE_API EventList::delAll()
{
    this->g_events.clear();
}

fge::Event* FGE_API EventList::get(const std::string& name)
{
    auto it = this->g_events.find(name);

    if ( it != this->g_events.end() )
    {
        return &it->second;
    }
    return nullptr;
}

}//end fge
