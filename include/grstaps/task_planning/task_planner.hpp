#ifndef GRSTAPS_TASK_PLANNER_HPP
#define GRSTAPS_TASK_PLANNER_HPP

// global
#include <memory>

// external
#include "nlohmann/json.hpp"

// local
#include "grstaps/noncopyable.hpp"
#include "grstaps/task_planning/selector.hpp"
#include "grstaps/task_planning/task_planner_setting.hpp"
#include "grstaps/task_planning/utils.hpp"

namespace grstaps
{
    class Plan;
    class SASTask;
    class Successors;
    class TState;

    class TaskPlanner
        : public TaskPlannerSetting
        , Noncopyable
    {
       public:
        TaskPlanner(SASTask* m_task, float m_timeout = -1.0f, bool trace = false);
        Plan* poll();
        std::vector<Plan*> getNextSuccessors(Plan* base);
        void update(Plan* base, std::vector<Plan*>& successors);
        bool emptySearchSpace();
    };
}  // namespace grstaps

#endif  // GRSTAPS_TASK_PLANNER_HPP
