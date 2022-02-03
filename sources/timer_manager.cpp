#include "FastEngine/timer_manager.hpp"

#include <list>
#include <mutex>
#include <thread>
#include <condition_variable>

#include "FastEngine/C_clock.hpp"

namespace fge
{
namespace timer
{

namespace
{

using DataTimersType = std::list<fge::timer::TimerDataShared>;

DataTimersType __dataTimers;
std::mutex __dataMutex;
std::condition_variable __dataCv;

std::unique_ptr<std::thread> __timerThread;
bool __timerThreadRunning = false;

void TimerThread()
{
    std::chrono::milliseconds waitTime = std::chrono::milliseconds(1000);
    std::unique_lock<std::mutex> lckData(__dataMutex);

    fge::Clock clock;

    while (__timerThreadRunning)
    {
        __dataCv.wait_for(lckData, waitTime);

        if (!__timerThreadRunning)
        {
            break;
        }

        waitTime = std::chrono::milliseconds(1000);

        std::chrono::milliseconds interval = std::chrono::duration_cast<std::chrono::milliseconds>( clock.restart() );

        for (DataTimersType::iterator it=__dataTimers.begin(); it!=__dataTimers.end(); ++it)
        {
            fge::timer::TimerData* timerData = (*it).get();

            timerData->_timer.addToStep( interval );
            std::chrono::milliseconds timeLeft = timerData->_timer.getTimeLeft();
            if ( timeLeft.count() <= 0 )
            {//time reached
                timerData->_onTimeReached.call(timerData->_timer);

                timeLeft = timerData->_timer.getTimeLeft();
                if ( timeLeft.count() <= 0 )
                {//We recheck if the goal is reached after the call to the callback (if the user restarted the timer, we will not destroy it (loop))
                    it = __dataTimers.erase(it);
                    --it;
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

void FGE_API Init()
{
    if ( __timerThread == nullptr )
    {
        __timerThreadRunning = true;
        __timerThread.reset( new std::thread(TimerThread) );
    }
}
bool FGE_API IsInit()
{
    return __timerThreadRunning;
}
void FGE_API Uninit()
{
    if ( __timerThread != nullptr )
    {
        __timerThreadRunning = false;
        __dataCv.notify_all();

        __timerThread->join();
        __timerThread.reset(nullptr);

        __dataMutex.lock();
        __dataTimers.clear();
        __dataMutex.unlock();
    }
}

void FGE_API Notify()
{
    __dataCv.notify_all();
}

fge::timer::TimerDataShared FGE_API Create(const fge::Timer& timer)
{
    std::lock_guard<std::mutex> lck(__dataMutex);
    __dataTimers.push_back( std::move(std::make_shared<fge::timer::TimerData>(timer)) );
    __dataCv.notify_all();
    return __dataTimers.back();
}

bool FGE_API Destroy(const fge::timer::TimerDataShared& timer)
{
    std::lock_guard<std::mutex> lck(__dataMutex);
    for (DataTimersType::iterator it=__dataTimers.begin(); it!=__dataTimers.end(); ++it)
    {
        if ( (*it).get() == timer.get() )
        {
            __dataTimers.erase(it);
            __dataCv.notify_all();
            return true;
        }
    }
    return false;
}
bool FGE_API Destroy(const std::string& timerName)
{
    std::lock_guard<std::mutex> lck(__dataMutex);
    for (DataTimersType::iterator it=__dataTimers.begin(); it!=__dataTimers.end(); ++it)
    {
        if ( (*it)->_timer.getName() == timerName )
        {
            __dataTimers.erase(it);
            __dataCv.notify_all();
            return true;
        }
    }
    return false;
}

void FGE_API DestroyAll()
{
    std::lock_guard<std::mutex> lck(__dataMutex);
    __dataTimers.clear();
    __dataCv.notify_all();
}

bool FGE_API Check(const fge::timer::TimerDataShared& timer)
{
    std::lock_guard<std::mutex> lck(__dataMutex);
    for (DataTimersType::iterator it=__dataTimers.begin(); it!=__dataTimers.end(); ++it)
    {
        if ( (*it).get() == timer.get() )
        {
            return true;
        }
    }
    return false;
}
bool FGE_API Check(const std::string& timerName)
{
    std::lock_guard<std::mutex> lck(__dataMutex);
    for (DataTimersType::iterator it=__dataTimers.begin(); it!=__dataTimers.end(); ++it)
    {
        if ( (*it)->_timer.getName() == timerName )
        {
            return true;
        }
    }
    return false;
}

std::size_t FGE_API GetTimerSize()
{
    std::lock_guard<std::mutex> lck(__dataMutex);
    return __dataTimers.size();
}

fge::timer::TimerDataShared FGE_API Get(const std::string& timerName)
{
    std::lock_guard<std::mutex> lck(__dataMutex);
    for (DataTimersType::iterator it=__dataTimers.begin(); it!=__dataTimers.end(); ++it)
    {
        if ( (*it)->_timer.getName() == timerName )
        {
            return (*it);
        }
    }
    return nullptr;
}

}//end timer
}//end fge
