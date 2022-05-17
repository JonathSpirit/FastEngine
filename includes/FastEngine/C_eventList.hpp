#ifndef _FGE_C_EVENTLIST_HPP_INCLUDED
#define _FGE_C_EVENTLIST_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <FastEngine/C_event.hpp>
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

}//end fge

#endif // _FGE_C_EVENTLIST_HPP_INCLUDED
