#include "grstaps/solver_fcpop.hpp"

#include "grstaps/Task_Allocation/TaskAllocation.h"
#include "grstaps/task_planning/planner_parameters.hpp"
#include "grstaps/task_planning/task_planner.hpp"
#include "grstaps/task_planning/setup.hpp"
#include "grstaps/logger.hpp"
#include "grstaps/problem.hpp"
#include "grstaps/timer.hpp"

namespace grstaps
{
    void planSubcomponents(const Plan *base, std::vector<const Plan *> &plan_subcomponents)
    {
        if(base == nullptr)
        {
            plan_subcomponents.clear();
        }
        else
        {
            planSubcomponents(base->parentPlan, plan_subcomponents);
            plan_subcomponents.push_back(base);
        }
    }

    nlohmann::json writeSolution(TaskPlanner& task_planner, Plan* p, SASTask* task)
    {
        nlohmann::json rv;

        std::vector<const Plan*> plan_subcomponents;
        planSubcomponents(p, plan_subcomponents);
        std::vector<std::string> actions;
        for(const Plan* subcomponent: plan_subcomponents)
        {
            actions.push_back(subcomponent->action->name);
        }
        rv["actions"] = actions;
        rv["schedule"] = task_planner.scheduleAsJson(p, task);

        return rv;
    }

    nlohmann::json SolverFcpop::solve(const std::string& domain_filepath, const std::string& problem_filepath)
    {
        Logger::debug("start");
        // Preprocess pddl
        PlannerParameters parameters;
        parameters.domainFileName  = domain_filepath.c_str();
        parameters.problemFileName = problem_filepath.c_str();
//        {
//            parameters.outputFileName    = "problems/0/0/output";
            parameters.generateMutexFile = true;
            parameters.generateTrace = true;
            parameters.generateGroundedDomain = true;
//        }
        Timer timer;
        timer.start();
        auto task = Setup::doPreprocess(&parameters);
        timer.stop();
        float preprocess_time = timer.get();
        timer.reset();
        Logger::debug("Grounded Actions: {}", task->actions.size());

        // Task planner
        TaskPlanner task_planner(task);
        m_tp_nodes_expanded = 0;
        m_tp_nodes_visited  = 0;
        float num_branches = 0;
        float num_times_branched = 0;

        timer.start();
        Plan* base;
        while(!task_planner.emptySearchSpace())
        {
            base = task_planner.poll();
            if(base->isSolution())
            {
                timer.stop();
                nlohmann::json metrics = {
                    {"makespan", task_planner.getMakespan(base)},
                    {"total_grounded_actions", task->actions.size()},
                    {"num_actions", task_planner.getPlanActions(base)},
                    {"num_tp_nodes_expanded", m_tp_nodes_expanded},
                    {"num_tp_nodes_visited", m_tp_nodes_visited},
                    {"avg_branching_factor", num_branches / num_times_branched},
                    {"timer", timer.get()},
                    {"preprocess_timer", preprocess_time},
                    {"solution", writeSolution(task_planner, base, task)}
                };
                return metrics;
            }

            ++m_tp_nodes_expanded;
            Logger::debug("Expanding plan: {}", base->id);
            std::vector<Plan*> successors = task_planner.getNextSuccessors(base);
            ++num_times_branched;
            for(Plan* plan : successors)
            {
                m_tp_nodes_visited += successors.size();
                num_branches += successors.size();
                task_planner.update(base, successors);
            }
        }

        return nlohmann::json();
    }
}