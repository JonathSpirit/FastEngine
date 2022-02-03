#ifndef _FGE_C_EVENTLIST_HPP_INCLUDED
#define _FGE_C_EVENTLIST_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <FastEngine/C_event.hpp>
#include <unordered_map>

namespace fge
{

class FGE_API EventList
{
public:
    EventList() = default;
    ~EventList() = default;

    bool add(const std::string& name);
    bool del(const std::string& name);
    void delAll();

    fge::Event* get(const std::string& name);

private:
    std::unordered_map<std::string, fge::Event> g_events;
};

}//end fge

#endif // _FGE_C_EVENTLIST_HPP_INCLUDED
