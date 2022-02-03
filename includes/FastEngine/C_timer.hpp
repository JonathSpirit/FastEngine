#ifndef _FGE_C_TIMER_HPP_INCLUDED
#define _FGE_C_TIMER_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <chrono>
#include <string>
#include <mutex>

namespace fge
{

class FGE_API Timer
{
public:
    Timer(const fge::Timer& timer);
    Timer(fge::Timer&& timer);

    Timer(const std::chrono::milliseconds& goal);
    Timer(const std::chrono::milliseconds& goal, bool paused);
    Timer(const std::chrono::milliseconds& goal, const std::string& name, bool paused);

    void setName(const std::string& name);
    const std::string& getName() const;

    void setGoalDuration(const std::chrono::milliseconds& t);
    void addToGoal(const std::chrono::milliseconds& t);
    void subToGoal(const std::chrono::milliseconds& t);

    void setStepDuration(const std::chrono::milliseconds& t);
    void addToStep(const std::chrono::milliseconds& t);
    void subToStep(const std::chrono::milliseconds& t);

    const std::chrono::steady_clock::time_point& getLifeTimePoint() const;
    std::chrono::milliseconds getLifeDuration() const;

    const std::chrono::milliseconds& getStepDuration() const;
    const std::chrono::milliseconds& getGoalDuration() const;

    std::chrono::milliseconds getTimeLeft() const;

    bool goalReached() const;
    void restart();

    void pause();
    void resume();
    bool isPaused() const;

    std::mutex& getMutex();

private:
    const std::chrono::steady_clock::time_point g_lifeTimePoint;

    std::chrono::milliseconds g_stepDuration;
    std::chrono::milliseconds g_goalDuration;

    bool g_isPaused;

    std::string g_name;

    mutable std::mutex g_mutex;
};

}//end fge

#endif // _FGE_C_TIMER_HPP_INCLUDED
