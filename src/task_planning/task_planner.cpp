#include "grstaps/task_planning/task_planner.hpp"

namespace grstaps
{
    TaskPlanner::TaskPlanner(SASTask* task, float timeout, bool trace)
        : TaskPlannerSetting(task, trace, timeout)
    {}

    Plan* TaskPlanner::poll()
    {
        return m_planner->poll();
    }

    std::vector<Plan*> TaskPlanner::getNextSuccessors(Plan* base)
    {
        if(base == nullptr)
        {
            return std::vector<Plan*>();
        }
        return m_planner->getNextSuccessors(base);
    }

    void TaskPlanner::update(Plan* base, std::vector<Plan*>& successors)
    {
        m_planner->update(base, successors);
    }

    bool TaskPlanner::emptySearchSpace()
    {
        return m_planner->emptySearchSpace();
    }

    void TaskPlanner::writeTrace(std::ostream& f, Plan* p)
    {
        // if (numTracedPlans > 1000) return;	// Only 1000 expanded plans in the trace
        // at most
        f << "BASE PLAN" << std::endl;
        f << (p->id) << std::endl;
        f << "CHILDREN" << std::endl;
        unsigned int n = p->childPlans == nullptr ? 0 : p->childPlans->size();
        f << n << std::endl;
        for(unsigned int i = 0; i < n; i++)
        {
            f << "CHILD " << i << std::endl;
            Plan* s = p->childPlans->at(i);
            f << s->id << std::endl;
            f << planToPDDL(s);
            f << ";H: " << s->h << std::endl;
            f << s->toString();
            if(s->openCond != nullptr)
            {
                for(unsigned int i = 0; i < s->openCond->size(); i++)
                {
                    f << "  * OC: " << s->openCond->at(i).step << ":" << s->openCond->at(i).condNumber << std::endl;
                }
            }
            if(s->action != nullptr && s->action->isGoal && !s->unsatisfiedNumericConditions)
            {
                f << ";GOAL" << std::endl;
            }
        }
    }

    std::string TaskPlanner::planToPDDL(Plan* p)
    {
        Linearizer linearizer;
        linearizer.setInitialState(m_initial_state, m_task);
        return linearizer.planToPDDL(p, m_task);
    }
}  // namespace grstaps
