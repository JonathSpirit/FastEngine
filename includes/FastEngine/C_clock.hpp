#ifndef _FGE_C_CLOCK_HPP_INCLUDED
#define _FGE_C_CLOCK_HPP_INCLUDED

#include <chrono>

namespace fge
{

class Clock
{
public:
    Clock() :
        g_lastTimePoint( std::chrono::steady_clock::now() )
    {}

    auto getElapsedTime() const
    {
        return std::chrono::steady_clock::now() - this->g_lastTimePoint;
    }

    template<typename TD>
    auto getElapsedTime() const
    {
        return std::chrono::duration_cast<TD>(std::chrono::steady_clock::now() - this->g_lastTimePoint).count();
    }

    auto restart()
    {
        auto lastDuration = std::chrono::steady_clock::now() - this->g_lastTimePoint;
        this->g_lastTimePoint = std::chrono::steady_clock::now();
        return lastDuration;
    }

    template<typename TD>
    auto restart()
    {
        auto lastDuration = std::chrono::duration_cast<TD>(std::chrono::steady_clock::now() - this->g_lastTimePoint).count();
        this->g_lastTimePoint = std::chrono::steady_clock::now();
        return lastDuration;
    }

    bool reached(const std::chrono::milliseconds& duration)
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - this->g_lastTimePoint) >= duration;
    }

private:
    std::chrono::steady_clock::time_point g_lastTimePoint;
};

}//end fge

#endif // _FGE_C_CLOCK_HPP_INCLUDED
