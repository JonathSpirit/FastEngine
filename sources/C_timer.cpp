/*
 * Copyright 2024 Guillaume Guillet
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

#include "FastEngine/C_timer.hpp"

namespace fge
{

Timer::Timer(fge::Timer const& timer) :
        g_lifeTimePoint(timer.g_lifeTimePoint),

        g_elapsedTime(timer.g_elapsedTime),
        g_goalDuration(timer.g_goalDuration),

        g_isPaused(timer.g_isPaused),
        g_name(timer.g_name)
{}
Timer::Timer(fge::Timer&& timer) noexcept :
        g_lifeTimePoint(std::move(timer.g_lifeTimePoint)),

        g_elapsedTime(std::move(timer.g_elapsedTime)),
        g_goalDuration(std::move(timer.g_goalDuration)),

        g_isPaused(std::move(timer.g_isPaused)),
        g_name(std::move(timer.g_name))
{}

Timer::Timer(std::chrono::milliseconds const& goal) :
        g_lifeTimePoint(std::chrono::steady_clock::now()),

        g_elapsedTime(0),
        g_goalDuration(goal),

        g_isPaused(false),
        g_name()
{}
Timer::Timer(std::chrono::milliseconds const& goal, bool paused) :
        g_lifeTimePoint(std::chrono::steady_clock::now()),

        g_elapsedTime(0),
        g_goalDuration(goal),

        g_isPaused(paused),
        g_name()
{}
Timer::Timer(std::chrono::milliseconds const& goal, std::string name, bool paused) :
        g_lifeTimePoint(std::chrono::steady_clock::now()),

        g_elapsedTime(0),
        g_goalDuration(goal),

        g_isPaused(paused),
        g_name(std::move(name))
{}

void Timer::setName(std::string const& name)
{
    std::scoped_lock<std::mutex> const lck(this->g_mutex);
    this->g_name = name;
}
std::string const& Timer::getName() const
{
    std::scoped_lock<std::mutex> const lck(this->g_mutex);
    return this->g_name;
}

void Timer::setGoalDuration(std::chrono::milliseconds const& t)
{
    std::scoped_lock<std::mutex> const lck(this->g_mutex);
    this->g_goalDuration = t;
}
void Timer::addToGoal(std::chrono::milliseconds const& t)
{
    std::scoped_lock<std::mutex> const lck(this->g_mutex);
    this->g_goalDuration += t;
}
void Timer::subToGoal(std::chrono::milliseconds const& t)
{
    std::scoped_lock<std::mutex> const lck(this->g_mutex);
    this->g_goalDuration -= t;
}

void Timer::setElapsedTime(std::chrono::milliseconds const& t)
{
    std::scoped_lock<std::mutex> const lck(this->g_mutex);
    if (!this->g_isPaused)
    {
        this->g_elapsedTime = t;
    }
}
void Timer::addToElapsedTime(std::chrono::milliseconds const& t)
{
    std::scoped_lock<std::mutex> const lck(this->g_mutex);
    if (!this->g_isPaused)
    {
        this->g_elapsedTime += t;
    }
}
void Timer::subToElapsedTime(std::chrono::milliseconds const& t)
{
    std::scoped_lock<std::mutex> const lck(this->g_mutex);
    if (!this->g_isPaused)
    {
        this->g_elapsedTime -= t;
    }
}

std::chrono::steady_clock::time_point const& Timer::getLifeTimePoint() const
{
    std::scoped_lock<std::mutex> const lck(this->g_mutex);
    return this->g_lifeTimePoint;
}
std::chrono::milliseconds Timer::getLifeDuration() const
{
    std::scoped_lock<std::mutex> const lck(this->g_mutex);
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() -
                                                                 this->g_lifeTimePoint);
}

std::chrono::milliseconds const& Timer::getElapsedTime() const
{
    std::scoped_lock<std::mutex> const lck(this->g_mutex);
    return this->g_elapsedTime;
}
std::chrono::milliseconds const& Timer::getGoalDuration() const
{
    std::scoped_lock<std::mutex> const lck(this->g_mutex);
    return this->g_goalDuration;
}

std::chrono::milliseconds Timer::getTimeLeft() const
{
    std::scoped_lock<std::mutex> const lck(this->g_mutex);
    return this->g_goalDuration - this->g_elapsedTime;
}

bool Timer::goalReached() const
{
    std::scoped_lock<std::mutex> const lck(this->g_mutex);
    return (this->g_goalDuration - this->g_elapsedTime).count() <= 0;
}
void Timer::restart()
{
    std::scoped_lock<std::mutex> const lck(this->g_mutex);
    this->g_elapsedTime = std::chrono::milliseconds(0);
}

void Timer::pause()
{
    std::scoped_lock<std::mutex> const lck(this->g_mutex);
    this->g_isPaused = true;
}
void Timer::resume()
{
    std::scoped_lock<std::mutex> const lck(this->g_mutex);
    this->g_isPaused = false;
}
bool Timer::isPaused() const
{
    std::scoped_lock<std::mutex> const lck(this->g_mutex);
    return this->g_isPaused;
}

} // namespace fge
