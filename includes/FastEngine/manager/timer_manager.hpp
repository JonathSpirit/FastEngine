/*
 * Copyright 2025 Guillaume Guillet
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

#ifndef _FGE_TIMER_MANAGER_HPP_INCLUDED
#define _FGE_TIMER_MANAGER_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "FastEngine/C_callback.hpp"
#include "FastEngine/C_timer.hpp"
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
FGE_API bool Destroy(fge::timer::TimerShared const& timer);
/**
 * \brief Destroy a timer with the given name.
 *
 * \param timerName The timer name to destroy
 * \return \b true if the timer was destroyed, \b false otherwise
 */
FGE_API bool Remove(std::string const& timerName);

/**
 * \brief Destroy all timers
 */
FGE_API void RemoveAll();

/**
 * \brief Check if the given timer pointer exist in the manager.
 *
 * \param timer The timer to check
 * \return \b true if the timer exist, \b false otherwise
 */
FGE_API bool Check(fge::timer::TimerShared const& timer);
/**
 * \brief Check if the given timer name exist in the manager.
 *
 * \param timerName The timer name to check
 * \return \b true if the timer exist, \b false otherwise
 */
FGE_API bool Check(std::string const& timerName);

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
FGE_API fge::timer::TimerShared Get(std::string const& timerName);

/**
 * @}
 */

} // namespace fge::timer

#endif // _FGE_TIMER_MANAGER_HPP_INCLUDED
