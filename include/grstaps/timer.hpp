#ifndef __GRSTAPS_TIMER_H__
#define __GRSTAPS_TIMER_H__

// Global
#include <chrono>


// External
#include <nlohmann/json.hpp>

namespace grstaps
{
    //! \brief A simple timer
    class Timer
    {
       public:
        //! \brief Constructor
        Timer();

        //! \brief Starts the timer
        void start();

        //! \brief Stops the timer
        void stop();

        //! \brief Resets the timer
        void reset();

        //! \returns Whether the timer is running
        [[nodiscard]] bool isRunning() const;

        //! \retuns The value from the timer in seconds
        [[nodiscard]] float get() const;

       private:
        bool m_running;
        std::chrono::time_point<std::chrono::system_clock> m_start_time;
        float m_time; //!< seconds
    };
}
#endif // __GRSTAPS_TIMER_H__
