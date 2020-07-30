#ifndef GRSTAPS_TASK_PLANNER_HPP
#define GRSTAPS_TASK_PLANNER_HPP

// global
#include <memory>

// external
#include "nlohmann/json.hpp"

// local
#include "grstaps/noncopyable.hpp"
#include "grstaps/task_planning/selector.hpp"
#include "grstaps/task_planning/utils.hpp"

namespace grstaps
{
    class Plan;
    class SASTask;
    class Successors;
    class TState;

    class TaskPlanner : public Noncopyable
    {
       private:
        SASTask* m_task;
        Plan* m_initial_plan;
        TState* m_initial_state;
        unsigned int m_expanded_nodes;
        Successors* m_successors;
        float m_initial_h;
        QualitySelector m_quality_selector;
        float m_timeout;
        clock_t m_start_time;

        SASAction* createInitialAction();
        SASAction* createFictitiousAction(float actionDuration,
                                          std::vector<unsigned int>& varList,
                                          float timePoint,
                                          std::string name,
                                          bool isTIL);
        void addFrontierNodes(Plan* p);

       public:
        TaskPlanner(SASTask* task, float timeout = -1.0f);
        Plan* bestPlan();
        std::vector<Plan*> getNextSuccessors(Plan* base);
        void update(Plan* base, std::vector<Plan*>& successors);
        bool emptySearchSpace();
    };
}  // namespace grstaps

#endif  // GRSTAPS_TASK_PLANNER_HPP
