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

#ifndef _FGE_C_TIMER_HPP_INCLUDED
#define _FGE_C_TIMER_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "FastEngine/C_callback.hpp"
#include <chrono>
#include <mutex>
#include <string>

namespace fge
{

/**
 * \class Timer
 * \ingroup time
 * \brief A timer that can be used with the timer manager to handle time.
 */
class FGE_API Timer
{
public:
    Timer(fge::Timer const& timer);
    Timer(fge::Timer&& timer) noexcept;

    /**
     * \brief Create a new timer with the given time goal.
     *
     * \param goal The time goal of the timer
     */
    explicit Timer(std::chrono::milliseconds const& goal);
    /**
     * \brief Create a new timer with the given time goal and pause state.
     *
     * \param goal The time goal of the timer
     * \param paused The pause state of the timer
     */
    Timer(std::chrono::milliseconds const& goal, bool paused);
    /**
     * \brief Create a new timer with the given time goal, name and pause state.
     *
     * \param goal The time goal of the timer
     * \param name The name of the timer
     * \param paused The pause state of the timer
     */
    Timer(std::chrono::milliseconds const& goal, std::string name, bool paused);

    /**
     * \brief Set the timer name
     *
     * \param name The name of the timer
     */
    void setName(std::string const& name);
    /**
     * \brief Get the timer name
     *
     * \return The name of the timer
     */
    std::string const& getName() const;

    /**
     * \brief Set the goal duration of the timer
     *
     * This is the time goal of the timer, once reached the timer will be stopped and
     * an callback will be called, you will be able to restart the timer in this callback.
     *
     * \param t The goal duration of the timer
     */
    void setGoalDuration(std::chrono::milliseconds const& t);
    /**
     * \brief Add an amount of time to the goal duration of the timer
     *
     * \see setGoalDuration
     *
     * \param t The amount of time to add to the goal duration
     */
    void addToGoal(std::chrono::milliseconds const& t);
    /**
     * \brief Subtract an amount of time to the goal duration of the timer
     *
     * \see setGoalDuration
     *
     * \param t The amount of time to subtract to the goal duration
     */
    void subToGoal(std::chrono::milliseconds const& t);

    /**
     * \brief Set the elapsed time of the timer
     *
     * \param t The elapsed time of the timer
     */
    void setElapsedTime(std::chrono::milliseconds const& t);
    /**
     * \brief Add an amount of time to the elapsed time of the timer
     *
     * \param t The amount of time to add to the elapsed time
     */
    void addToElapsedTime(std::chrono::milliseconds const& t);
    /**
     * \brief Subtract an amount of time to the elapsed time of the timer
     *
     * \param t The amount of time to subtract to the elapsed time
     */
    void subToElapsedTime(std::chrono::milliseconds const& t);

    /**
     * \brief Get the life time of the timer
     *
     * This is the time elapsed since the timer was created.
     *
     * \return The life time of the timer
     */
    std::chrono::steady_clock::time_point const& getLifeTimePoint() const;
    /**
     * \brief Get the life time of the timer as milliseconds
     *
     * \return The life time of the timer as milliseconds
     */
    std::chrono::milliseconds getLifeDuration() const;

    /**
     * \brief Get the elapsed time of the timer
     *
     * \return The elapsed time of the timer
     */
    std::chrono::milliseconds const& getElapsedTime() const;
    /**
     * \brief Get the goal time of the timer
     *
     * \return The goal time of the timer
     */
    std::chrono::milliseconds const& getGoalDuration() const;

    /**
     * \brief Get the remaining time of the timer
     *
     * \return The remaining time of the timer
     */
    std::chrono::milliseconds getTimeLeft() const;

    /**
     * \brief Check if the goal has been reached
     *
     * \return \b true if the goal has been reached, \b false otherwise
     */
    bool goalReached() const;
    /**
     * \brief Restart the timer
     *
     * This will reset the elapsed time to 0.
     */
    void restart();

    /**
     * \brief Pause the timer
     */
    void pause();
    /**
     * \brief Unpause the timer
     */
    void resume();
    /**
     * \brief Check if the timer is paused
     *
     * \return \b true if the timer is paused, \b false otherwise
     */
    bool isPaused() const;

    fge::CallbackHandler<fge::Timer&> _onTimeReached; ///< The callback called when the timer reaches the goal

private:
    std::chrono::steady_clock::time_point const g_lifeTimePoint;

    std::chrono::milliseconds g_elapsedTime;
    std::chrono::milliseconds g_goalDuration;

    bool g_isPaused;

    std::string g_name;

    mutable std::mutex g_mutex;
};

} // namespace fge

#endif // _FGE_C_TIMER_HPP_INCLUDED
