#include <grstaps/timer.h>

// Global
#include <sstream>

namespace grstaps
{
    std::vector<std::pair<double, Timer::SplitType> > Timer::s_timer_splits;
    Timer::TimerType Timer::s_type = Timer::TimerType::e_grstaps;
    float Timer::s_tplan_time    = 0;
    float Timer::s_talloc_time   = 0;
    float Timer::s_schedule_time = 0;
    float Timer::s_mp_time       = 0;

    Timer::Timer()
        : m_running(false)
    {}

    void Timer::setITAGS()
    {
        s_type = TimerType::e_itags;
    }

    void Timer::setITAGS_S()
    {
        s_type = TimerType::e_itags_s;
    }

    void Timer::start()
    {
        if(!m_running)
        {
            m_start_time = std::chrono::system_clock::now();
            m_running    = true;
        }
    }

    void Timer::stop()
    {
        if(m_running)
        {
            m_end_time = std::chrono::system_clock::now();
            m_running  = false;
        }
    }

    void Timer::reset()
    {
        this->m_running = false;
    }

    void Timer::restart()
    {
        if(!m_running)
        {
            start();
        }
    }

    void Timer::recordSplit(Timer::SplitType t)
    {
        s_timer_splits.push_back(std::pair<double, SplitType>(getTime(), t));
    }

    double Timer::getTime()
    {
        if(!m_running)
        {
            return std::chrono::duration_cast<std::chrono::milliseconds>(m_end_time - m_start_time).count() * 1.0E-9;
        }
        else
        {
            std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
            return std::chrono::duration_cast<std::chrono::milliseconds>(now - m_start_time).count() * 1.0E-9;
        }
    }

    void Timer::calcSplits()
    {
        s_schedule_time = 0;
        s_talloc_time   = 0;
        s_tplan_time    = 0;
        s_mp_time       = 0;

        for(auto iter : s_timer_splits)
        {
            double curr_split = iter.first;
            SplitType type   = iter.second;
            if(s_type == TimerType::e_grstaps)
            {
                switch(type)
                {
                    case SplitType::e_tp:
                        s_tplan_time += curr_split;
                        break;
                    case SplitType::e_ta:
                        s_talloc_time += curr_split;
                        break;
                    case SplitType::e_s:
                        s_schedule_time += curr_split;
                        break;
                    case SplitType::e_mp:
                        s_mp_time += curr_split;
                        break;
                }
            }
            else if (s_type == TimerType::e_itags || s_type == TimerType::e_itags_s)
            {
                switch(type)
                {
                    case SplitType::e_ta:
                        s_talloc_time += curr_split;
                        break;
                    case SplitType::e_s:
                        s_schedule_time += curr_split;
                        break;
                    case SplitType::e_mp:
                        s_mp_time += curr_split;
                        break;
                    default:
                        throw "TP";
                }
            }
        }

        // ITAGS and ITAGS_S don't have planning
        if(s_type == TimerType::e_grstaps)
        {
            s_tplan_time -= s_talloc_time;
        }

        // ITAGS_S does TA separate from S&M
        if(s_type != TimerType::e_itags_s)
        {
            s_talloc_time -= s_schedule_time;
        }
        s_schedule_time -= s_mp_time;


    }
    void to_json(nlohmann::json& j, const Timer& t)
    {
        Timer::calcSplits();
        j["tp_compute_time"]    = t.s_tplan_time;
        j["ta_compute_time"]    = t.s_talloc_time;
        j["s_compute_time"]     = t.s_schedule_time;
        j["mp_compute_time"]    = t.s_mp_time;
        j["total_compute_time"] = t.s_tplan_time + t.s_talloc_time + t.s_schedule_time + t.s_mp_time;
    }
}  // namespace grstaps