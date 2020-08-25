//
// Created by glen on 7/20/20.
//

#include "grstaps/solution.hpp"

// external
#include <nlohmann/json.hpp>

// local
#include "grstaps/task_planning/plan.hpp"

namespace grstaps
{
    Solution::Solution(std::shared_ptr<Plan> plan, std::shared_ptr<TaskAllocation> allocation)
    {
        m_plan       = plan;
        m_allocation = allocation;
    }

    void Solution::write(const std::string& filepath)
    {
        std::vector<const Plan*> plan_subcomponents;
        planSubcomponents(m_plan.get(), plan_subcomponents);

        nlohmann::json j;

        std::set<std::pair<uint16_t, uint16_t>> ordering_constraints;

        // Ignore #initial and <goal>
        for(unsigned int i = 1; i < plan_subcomponents.size() - 1; ++i)
        {
            const Plan* p = plan_subcomponents[i];
            j["schedule"].push_back({{"name", p->action->name},
                                     {"index", i},
                                     {"start_time", m_allocation->taToScheduling->sched.stn[i][0]},
                                     {"end_time", m_allocation->taToScheduling->sched.stn[i][1]}});

            for(unsigned int j = 0; j < plan_subcomponents[i]->orderings.size(); ++j)
            {
                // uint16_t
                TTimePoint fp = firstPoint(p->orderings[j]);
                TTimePoint sp = secondPoint(p->orderings[j]);
                // Time points are based on start and end snap actions
                // Also include the initial action

                ordering_constraints.insert({fp / 2 - 1, sp / 2 - 1});
            }
        }

        j["ordering_constaints"] = ordering_constraints;

        j["allocation"] = m_allocation->getID();

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
