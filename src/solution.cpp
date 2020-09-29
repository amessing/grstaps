//
// Created by glen on 7/20/20.
//

#include "grstaps/solution.hpp"

// global
#include <set>

// external
#include <fmt/format.h>

#include <nlohmann/json.hpp>

// local
#include "grstaps/Connections/taskAllocationToScheduling.h"
#include "grstaps/Task_Allocation/TaskAllocation.h"
#include "grstaps/motion_planning/motion_planner.hpp"
#include "grstaps/task_planning/plan.hpp"

namespace grstaps
{
    Solution::Solution(std::shared_ptr<Plan> plan,
                       std::shared_ptr<TaskAllocation> allocation,
                       const nlohmann::json& metrics)
        : m_plan(plan)
        , m_allocation(allocation)
        , m_metrics(metrics)
    {}

    void Solution::write(const std::string& filepath)
    {
        std::vector<const Plan*> plan_subcomponents;
        planSubcomponents(m_plan.get(), plan_subcomponents);

        nlohmann::json j;

        std::set<std::pair<uint16_t, uint16_t>> ordering_constraints;

        // Ignore #initial and <goal>
        j["schedule"]           = nlohmann::json();
        const auto motion_plans = m_allocation->taToScheduling.saveMotionPlanningNonSpeciesSchedule(m_allocation.get());
        if(motion_plans.first)
        {
            for(unsigned int i = 1; i < plan_subcomponents.size() - 1; ++i)
            {
                const unsigned int index = i - 1;
                const Plan* p            = plan_subcomponents[i];
                nlohmann::json action    = {{"name", p->action->name},
                                         {"index", index},
                                         {"start_time", m_allocation->taToScheduling.sched.actionStartTimes[index]},
                                         {"end_time", m_allocation->taToScheduling.sched.stn[index][1]}};
                j["schedule"].push_back(action);

                for(unsigned int j = 0; j < p->orderings.size(); ++j)
                {
                    // uint16_t
                    TTimePoint fp = firstPoint(p->orderings[j]);
                    TTimePoint sp = secondPoint(p->orderings[j]);
                    // Time points are based on start and end snap actions
                    // Also include the initial action

                    ordering_constraints.insert({fp / 2 - 1, sp / 2 - 1});
                }
            }

            j["ordering_constraints"] = ordering_constraints;

            j["allocation"] = m_allocation->getID();

            j["metrics"] = m_metrics;

            j["makespan"] = m_allocation->taToScheduling.sched.getMakeSpan();

            j["motion_plans"] = nlohmann::json();

            unsigned int i = 0;
            for(const auto& agent_motion_plans: motion_plans.second)
            {
                nlohmann::json agent;
                for(const auto& motion_plan: agent_motion_plans)
                {
                    nlohmann::json mp;
                    mp["start"]     = motion_plan.first.first;
                    mp["end"]       = motion_plan.first.second;
                    mp["waypoints"] = nlohmann::json();
                    for(const std::pair<float, float>& waypoint: motion_plan.second)
                    {
                        nlohmann::json w;
                        w.push_back(waypoint.first);
                        w.push_back(waypoint.second);
                        mp["waypoints"].push_back(w);
                    }
                    agent.push_back(mp);
                }
                j["motion_plans"][fmt::format("agent_{}", i)] = agent;
                ++i;
            }
        }

        std::ofstream output;
        output.open(filepath.c_str());
        output << j.dump(4);
    }

    const Plan& Solution::plan() const
    {
        return *m_plan;
    }

    const TaskAllocation& Solution::allocation() const
    {
        return *m_allocation;
    }

    const nlohmann::json& Solution::metrics() const
    {
        return m_metrics;
    }

    void Solution::planSubcomponents(Plan* base, std::vector<const Plan*>& plan_subcomponents)
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

}  // namespace grstaps
