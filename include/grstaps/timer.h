#ifndef __GRSTAPS_TIMER_H__
#define __GRSTAPS_TIMER_H__

// Global
#include <chrono>
#include <vector>

// External
#include <nlohmann/json.hpp>

namespace grstaps
{
    class Timer
    {
       public:
        enum class SplitType
        {
            e_tp,
            e_ta,
            e_s,
            e_mp
        };

        enum class TimerType
        {
            e_grstaps,
            e_itags,
            e_itags_s
        };

        //! \brief Constructor
        Timer();

        static void setITAGS();

        static void setITAGS_S();

        //! \brief Start the Timer
        void start();

        //! \brief Stop the Timer
        void stop();

        //! \brief Restart the Timer
        void restart();

        void reset();

        void recordSplit(SplitType t);
        double getTime();
        static void printSplits();
        static void calcSplits();

        static TimerType s_type;
        static float s_schedule_time;
        static float s_talloc_time;
        static float s_tplan_time;
        static float s_mp_time;

       private:
        std::chrono::time_point<std::chrono::system_clock> m_start_time;
        std::chrono::time_point<std::chrono::system_clock> m_end_time;
        bool m_running;
        static std::vector<std::pair<double, SplitType> > s_timer_splits;
    };

    void to_json(nlohmann::json& j, const Timer& t);
}

#endif // __GRSTAPS_TIMER_H__
