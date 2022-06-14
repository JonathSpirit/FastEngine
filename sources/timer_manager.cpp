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

#include "FastEngine/timer_manager.hpp"

#include <list>
#include <thread>
#include <condition_variable>

#include "FastEngine/C_clock.hpp"

namespace fge::timer
{

namespace
{

using DataTimersType = std::list<fge::timer::TimerShared>;

DataTimersType _dataTimers;
std::mutex _dataMutex;
std::condition_variable _dataCv;

std::unique_ptr<std::thread> _timerThread;
bool _timerThreadRunning = false;

void TimerThread()
{
    std::chrono::milliseconds waitTime = std::chrono::milliseconds(1000);
    std::unique_lock<std::mutex> lckData(_dataMutex);

    fge::Clock clock;

    while (_timerThreadRunning)
    {
        _dataCv.wait_for(lckData, waitTime);

        if (!_timerThreadRunning)
        {
            break;
        }

        waitTime = std::chrono::milliseconds(1000);

        std::chrono::milliseconds interval = std::chrono::duration_cast<std::chrono::milliseconds>( clock.restart() );

        for (auto it=_dataTimers.begin(); it != _dataTimers.end(); ++it)
        {
            auto* timerData = (*it).get();

            timerData->addToElapsedTime(interval);
            std::chrono::milliseconds timeLeft = timerData->getTimeLeft();
            if ( timeLeft.count() <= 0 )
            {//time reached
                timerData->_onTimeReached.call(*timerData);

                timeLeft = timerData->getTimeLeft();
                if ( timeLeft.count() <= 0 )
                {//We recheck if the goal is reached after the call to the callback (if the user restarted the timer, we will not destroy it (loop))
                    it = --_dataTimers.erase(it);
                }
                else
                {//New goal
                    if (timeLeft < waitTime)
                    {
                        waitTime = timeLeft;
                    }
                }
            }
            else
            {
                if (timeLeft < waitTime)
                {
                    waitTime = timeLeft;
                }
            }
        }
    }
}

}//end

void Init()
{
    if (_timerThread == nullptr )
    {
        _timerThreadRunning = true;
        _timerThread = std::make_unique<std::thread>(TimerThread);
    }
}
bool IsInit()
{
    return _timerThreadRunning;
}
void Uninit()
{
    if (_timerThread != nullptr )
    {
        _timerThreadRunning = false;
        _dataCv.notify_all();

        _timerThread->join();
        _timerThread.reset(nullptr);

        _dataMutex.lock();
        _dataTimers.clear();
        _dataMutex.unlock();
    }
}

void Notify()
{
    _dataCv.notify_all();
}

fge::timer::TimerShared Create(fge::timer::TimerShared timer)
{
    std::lock_guard<std::mutex> lck(_dataMutex);
    _dataTimers.push_back( std::move(timer) );
    _dataCv.notify_all();
    return _dataTimers.back();
}

bool Destroy(const fge::timer::TimerShared& timer)
{
    std::lock_guard<std::mutex> lck(_dataMutex);
    for (auto it=_dataTimers.begin(); it != _dataTimers.end(); ++it)
    {
        if ( (*it).get() == timer.get() )
        {
            _dataTimers.erase(it);
            _dataCv.notify_all();
            return true;
        }
    }
    return false;
}
bool Destroy(const std::string& timerName)
{
    std::lock_guard<std::mutex> lck(_dataMutex);
    for (auto it=_dataTimers.begin(); it != _dataTimers.end(); ++it)
    {
        if ( (*it)->getName() == timerName )
        {
            _dataTimers.erase(it);
            _dataCv.notify_all();
            return true;
        }
    }
    return false;
}

void DestroyAll()
{
    std::lock_guard<std::mutex> lck(_dataMutex);
    _dataTimers.clear();
    _dataCv.notify_all();
}

bool Check(const fge::timer::TimerShared& timer)
{
    std::lock_guard<std::mutex> lck(_dataMutex);
    for (auto& dataTimer : _dataTimers)
    {
        if ( dataTimer.get() == timer.get() )
        {
            return true;
        }
    }
    return false;
}
bool Check(const std::string& timerName)
{
    std::lock_guard<std::mutex> lck(_dataMutex);
    for (auto& dataTimer : _dataTimers)
    {
        if ( dataTimer->getName() == timerName )
        {
            return true;
        }
    }
    return false;
}

std::size_t GetTimerSize()
{
    std::lock_guard<std::mutex> lck(_dataMutex);
    return _dataTimers.size();
}

fge::timer::TimerShared Get(const std::string& timerName)
{
    std::lock_guard<std::mutex> lck(_dataMutex);
    for (auto& dataTimer : _dataTimers)
    {
        if ( dataTimer->getName() == timerName )
        {
            return dataTimer;
        }
    }
    return nullptr;
}

}//end fge::timer
