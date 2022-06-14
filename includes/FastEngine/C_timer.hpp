#ifndef _FGE_C_TIMER_HPP_INCLUDED
#define _FGE_C_TIMER_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <FastEngine/C_callback.hpp>
#include <chrono>
#include <string>
#include <mutex>

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
    Timer(const fge::Timer& timer);
    Timer(fge::Timer&& timer) noexcept;

    /**
     * \brief Create a new timer with the given time goal.
     *
     * \param goal The time goal of the timer
     */
    explicit Timer(const std::chrono::milliseconds& goal);
    /**
     * \brief Create a new timer with the given time goal and pause state.
     *
     * \param goal The time goal of the timer
     * \param paused The pause state of the timer
     */
    Timer(const std::chrono::milliseconds& goal, bool paused);
    /**
     * \brief Create a new timer with the given time goal, name and pause state.
     *
     * \param goal The time goal of the timer
     * \param name The name of the timer
     * \param paused The pause state of the timer
     */
    Timer(const std::chrono::milliseconds& goal, std::string name, bool paused);

    /**
     * \brief Set the timer name
     *
     * \param name The name of the timer
     */
    void setName(const std::string& name);
    /**
     * \brief Get the timer name
     *
     * \return The name of the timer
     */
    const std::string& getName() const;

    /**
     * \brief Set the goal duration of the timer
     *
     * This is the time goal of the timer, once reached the timer will be stopped and
     * an callback will be called, you will be able to restart the timer in this callback.
     *
     * \param t The goal duration of the timer
     */
    void setGoalDuration(const std::chrono::milliseconds& t);
    /**
     * \brief Add an amount of time to the goal duration of the timer
     *
     * \see setGoalDuration
     *
     * \param t The amount of time to add to the goal duration
     */
    void addToGoal(const std::chrono::milliseconds& t);
    /**
     * \brief Subtract an amount of time to the goal duration of the timer
     *
     * \see setGoalDuration
     *
     * \param t The amount of time to subtract to the goal duration
     */
    void subToGoal(const std::chrono::milliseconds& t);

    /**
     * \brief Set the elapsed time of the timer
     *
     * \param t The elapsed time of the timer
     */
    void setElapsedTime(const std::chrono::milliseconds& t);
    /**
     * \brief Add an amount of time to the elapsed time of the timer
     *
     * \param t The amount of time to add to the elapsed time
     */
    void addToElapsedTime(const std::chrono::milliseconds& t);
    /**
     * \brief Subtract an amount of time to the elapsed time of the timer
     *
     * \param t The amount of time to subtract to the elapsed time
     */
    void subToElapsedTime(const std::chrono::milliseconds& t);

    /**
     * \brief Get the life time of the timer
     *
     * This is the time elapsed since the timer was created.
     *
     * \return The life time of the timer
     */
    const std::chrono::steady_clock::time_point& getLifeTimePoint() const;
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
    const std::chrono::milliseconds& getElapsedTime() const;
    /**
     * \brief Get the goal time of the timer
     *
     * \return The goal time of the timer
     */
    const std::chrono::milliseconds& getGoalDuration() const;

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
    const std::chrono::steady_clock::time_point g_lifeTimePoint;

    std::chrono::milliseconds g_elapsedTime;
    std::chrono::milliseconds g_goalDuration;

    bool g_isPaused;

    std::string g_name;

    mutable std::mutex g_mutex;
};

}//end fge

#endif // _FGE_C_TIMER_HPP_INCLUDED
