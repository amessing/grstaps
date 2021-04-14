#ifndef GRSTAPS_SOLVER_FCPOP_HPP
#define GRSTAPS_SOLVER_FCPOP_HPP

// external
#include <nlohmann/json.hpp>

namespace grstaps
{
    class SolverFcpop
    {
       public:
        nlohmann::json solve(const std::string& domain_filepath, const std::string& problem_filepath);
       private:
        unsigned int m_tp_nodes_expanded;
        unsigned int m_tp_nodes_visited;
    };
}
#endif  // GRSTAPS_SOLVER_FCPOP_HPP
