#include <grstaps/Timer.h>

// Global
#include <sstream>

namespace grstaps
{
    std::vector<std::pair<double, Timer::SplitType> > Timer::s_timer_splits;
    float Timer::s_tplan_time    = 0;
    float Timer::s_talloc_time   = 0;
    float Timer::s_schedule_time = 0;
    float Timer::s_mp_time       = 0;

    Timer::Timer()
        : m_running(false)
    {}

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

    /*
    void Timer::printSplits()
    {
        int msg_length, num_whitespaces;

        for(auto iter : s_timer_splits)
        {
            std::stringstream formatted_msg;

            double curr_split = iter.first;
            SplitType type   = iter.second;
            msg_length = strlen(curr_msg);

            // Num whitespaces for formatting is:
            // 80 max char - 13 char for logger - 13 for time - msg length
            num_whitespaces = 80 - 13 - 11 - msg_length - 3;

            formatted_msg << curr_msg;

            // Add periods to format message to 80 characters length
            for(int i = 0; i < num_whitespaces; i++)
                formatted_msg << ".";

            printf("%s%.7f sec \n", formatted_msg.str().c_str(), curr_split);
        }
    }
    */

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

        s_tplan_time -= s_talloc_time;
        s_talloc_time -= s_schedule_time;
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