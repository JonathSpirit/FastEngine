#ifndef _FGE_TIMER_MANAGER_HPP_INCLUDED
#define _FGE_TIMER_MANAGER_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <FastEngine/C_timer.hpp>
#include <FastEngine/C_callback.hpp>
#include <memory>
#include <string>

namespace fge
{
namespace timer
{

struct TimerData
{
    TimerData(const fge::Timer& timer) :
        _timer(timer)
    {
    }

    fge::Timer _timer;
    fge::CallbackHandler<fge::Timer&> _onTimeReached;
};
using TimerDataShared = std::shared_ptr<fge::timer::TimerData>;

void FGE_API Init();
bool FGE_API IsInit();
void FGE_API Uninit();

void FGE_API Notify();

fge::timer::TimerDataShared FGE_API Create(const fge::Timer& timer);

bool FGE_API Destroy(const fge::timer::TimerDataShared& timer);
bool FGE_API Destroy(const std::string& timerName);

void FGE_API DestroyAll();

bool FGE_API Check(const fge::timer::TimerDataShared& timer);
bool FGE_API Check(const std::string& timerName);

std::size_t FGE_API GetTimerSize();

fge::timer::TimerDataShared FGE_API Get(const std::string& timerName);

}//end timer
}//end fge

#endif // _FGE_TIMER_MANAGER_HPP_INCLUDED
