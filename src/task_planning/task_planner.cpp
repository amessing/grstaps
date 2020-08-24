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
}  // namespace grstaps
