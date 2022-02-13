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

FGE_API void Init();
FGE_API bool IsInit();
FGE_API void Uninit();

FGE_API void Notify();

FGE_API fge::timer::TimerDataShared Create(const fge::Timer& timer);

FGE_API bool Destroy(const fge::timer::TimerDataShared& timer);
FGE_API bool Destroy(const std::string& timerName);

FGE_API void DestroyAll();

FGE_API bool Check(const fge::timer::TimerDataShared& timer);
FGE_API bool Check(const std::string& timerName);

FGE_API std::size_t GetTimerSize();

FGE_API fge::timer::TimerDataShared Get(const std::string& timerName);

}//end timer
}//end fge

#endif // _FGE_TIMER_MANAGER_HPP_INCLUDED
