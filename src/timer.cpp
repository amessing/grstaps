#include "grstaps/timer.hpp"

// local
//#include "grstaps/custom_exception.hpp"
#include "grstaps/logger.hpp"

namespace grstaps
{
    //EXCEPTION(TimerStillRunningError)

    Timer::Timer()
        : m_running(false)
        , m_time(0.0F)
    {}

    void Timer::start()
    {
        if(!m_running)
        {
            m_start_time = std::chrono::system_clock::now();
            m_running = true;
        }
        else
        {
            Logger::warn("Timer::start called when already running");
        }
    }

    void Timer::stop()
    {
        if(m_running)
        {
            auto end_time = std::chrono::system_clock::now();
            m_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - m_start_time).count() * 1.0E-9F;
            m_running = false;
        }
        else
        {
            Logger::warn("Timer::stop called when not running");
        }
    }

    void Timer::reset()
    {
        if(m_running)
        {
            //CUSTOM_ERROR(TimerStillRunningError, "Timer still running during reset")
        }
        m_time = 0.0F;
    }

    bool Timer::isRunning() const
    {
        return m_running;
    }

    float Timer::get() const
    {
        if(m_running)
        {
            //CUSTOM_ERROR(TimerStillRunningError, "Timer still running during get")
        }
        return m_time;
    }

}  // namespace grstaps