#ifndef GRSTAPS_EXPERIMENTS_HPP
#define GRSTAPS_EXPERIMENTS_HPP

// Global
#include <memory>
#include <vector>

// GRSTAPS
#include <grstaps/timer.h>
#include <grstaps/Search/AStarSearch.h>
#include <grstaps/Task_Allocation/TaskAllocation.h>

// Local
#include "icra_problem_v1.hpp"
#include "old_icra_problem_v1.hpp"

namespace grstaps
{

    namespace icra2021
    {
        class Experiments : public Noncopyable
        {
           public:
            static Experiments& getInstance();

            void setProblemVersion(unsigned int version);

            /**
             * ITAGS solve
             */
            nlohmann::json solve(const unsigned int problem_number, const float alpha);

            /**
             * Sequential solve
             */
            nlohmann::json solveSequentially(const unsigned int problem_number);

            /**
             * Spit out data from solve attempt
             */
            nlohmann::json errorData();

            /**
             * Clears the data
             */
            void clear();

            /**
             * Handle SIGTERM for timeout
             *
             * @param signal
             */
            static void handleSignal(int signal);

           private:
            Experiments() = default;
            std::unique_ptr<IcraProblemBase> getProblem(const unsigned int problem_number) const;
            static std::vector<std::vector<std::vector<b2PolygonShape>>> readMaps(const unsigned int nr);
            nlohmann::json writeSolution(TaskAllocation& allocation) const;

            Timer m_timer;
            std::unique_ptr<AStarSearch<TaskAllocation>> m_search;
            unsigned int m_problem_number;
            unsigned int m_problem_version;
            float m_alpha;
            bool m_sequential;
        };
    }
}

#endif  // GRSTAPS_EXPERIMENTS_HPP
