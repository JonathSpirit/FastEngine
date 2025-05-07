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

#ifndef _FGE_C_CLOCK_HPP_INCLUDED
#define _FGE_C_CLOCK_HPP_INCLUDED

#include <chrono>

namespace fge
{

/**
 * \class Clock
 * \ingroup time
 * \brief A clock that can be used to measure time.
 *
 * This class use std::chrono::steady_clock to measure time.
 */
class Clock
{
public:
    /**
     * \brief Default constructor
     *
     * When a clock is created, it set his time point to the current time.
     */
    Clock() :
            g_lastTimePoint(std::chrono::steady_clock::now())
    {}

    /**
     * \brief Get the time elapsed since the last time point
     *
     * \return The time elapsed since the last time point
     */
    auto getElapsedTime() const { return std::chrono::steady_clock::now() - this->g_lastTimePoint; }

    /**
     * \brief Get the time elapsed since the last time point with duration cast to the given type
     *
     * \tparam TD The type of the chrono duration
     * \return The time elapsed since the last time point casted to the given type
     */
    template<typename TD>
    auto getElapsedTime() const
    {
        return std::chrono::duration_cast<TD>(std::chrono::steady_clock::now() - this->g_lastTimePoint).count();
    }

    /**
     * \brief Restart the clock and return the time elapsed since the last time point
     *
     * By restarting the clock, the time point is reset to the current time.
     *
     * \return The time elapsed since the last time point
     */
    auto restart()
    {
        auto lastDuration = std::chrono::steady_clock::now() - this->g_lastTimePoint;
        this->g_lastTimePoint = std::chrono::steady_clock::now();
        return lastDuration;
    }

    /**
     * \brief Restart the clock and return the time elapsed since the last time point with duration cast to the given type
     *
     * By restarting the clock, the time point is reset to the current time.
     *
     * \tparam TD The type of the chrono duration
     * \return The time elapsed since the last time point casted to the given type
     */
    template<typename TD>
    auto restart()
    {
        auto lastDuration =
                std::chrono::duration_cast<TD>(std::chrono::steady_clock::now() - this->g_lastTimePoint).count();
        this->g_lastTimePoint = std::chrono::steady_clock::now();
        return lastDuration;
    }

    /**
     * \brief Check if the clock has elapsed the given duration
     *
     * \param duration The chrono duration to check
     * \return \b True if the clock has elapsed the given duration, \b false otherwise
     */
    bool reached(std::chrono::milliseconds const& duration)
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() -
                                                                     this->g_lastTimePoint) >= duration;
    }

private:
    std::chrono::steady_clock::time_point g_lastTimePoint;
};

} // namespace fge

#endif // _FGE_C_CLOCK_HPP_INCLUDED
