#ifndef _FGE_TIMER_MANAGER_HPP_INCLUDED
#define _FGE_TIMER_MANAGER_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <FastEngine/C_timer.hpp>
#include <FastEngine/C_callback.hpp>
#include <memory>
#include <string>

namespace fge::timer
{

using TimerShared = std::shared_ptr<fge::Timer>;

/**
 * \ingroup time
 * @{
 */

/**
 * \brief Init the timer manager
 *
 * This will create the timer thread.
 */
FGE_API void Init();
/**
 * \brief Check if the timer manager is initialized
 *
 * \return \b true if the timer manager is initialized, \b false otherwise
 */
FGE_API bool IsInit();
/**
 * \brief Un-init the timer manager
 */
FGE_API void Uninit();

/**
 * \brief Notify the timer manager thread generally used after adding/updating a timer.
 */
FGE_API void Notify();

/**
 * \brief Add a new timer to be handled by the thread.
 *
 * \param timer The timer to add
 * \return The same timer pointer
 */
FGE_API fge::timer::TimerShared Create(fge::timer::TimerShared timer);

/**
 * \brief Destroy a timer with the given pointer.
 *
 * \param timer The timer to destroy
 * \return \b true if the timer was destroyed, \b false otherwise
 */
FGE_API bool Destroy(const fge::timer::TimerShared& timer);
/**
 * \brief Destroy a timer with the given name.
 *
 * \param timerName The timer name to destroy
 * \return \b true if the timer was destroyed, \b false otherwise
 */
FGE_API bool Destroy(const std::string& timerName);

/**
 * \brief Destroy all timers
 */
FGE_API void DestroyAll();

/**
 * \brief Check if the given timer pointer exist in the manager.
 *
 * \param timer The timer to check
 * \return \b true if the timer exist, \b false otherwise
 */
FGE_API bool Check(const fge::timer::TimerShared& timer);
/**
 * \brief Check if the given timer name exist in the manager.
 *
 * \param timerName The timer name to check
 * \return \b true if the timer exist, \b false otherwise
 */
FGE_API bool Check(const std::string& timerName);

/**
 * \brief Get the total number of timers
 *
 * \return The total number of timers
 */
FGE_API std::size_t GetTimerSize();

/**
 * \brief Get the timer pointer with the given name.
 *
 * \param timerName The name of the timer to get
 * \return The timer pointer with the given name or the \b nullptr if not found
 */
FGE_API fge::timer::TimerShared Get(const std::string& timerName);

/**
 * @}
 */

}//end fge::timer

#endif // _FGE_TIMER_MANAGER_HPP_INCLUDED
