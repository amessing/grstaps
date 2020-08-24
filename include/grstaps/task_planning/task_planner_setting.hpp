#ifndef PLANNER_SETTING_H
#define PLANNER_SETTING_H

#include <time.h>

#include "grstaps/task_planning/plan.hpp"
#include "grstaps/task_planning/sas_task.hpp"
#include "grstaps/task_planning/task_planner_base.hpp"

namespace grstaps
{
    class TaskPlannerSetting
    {
       protected:
        SASTask* m_task;
        bool m_generate_trace;
        bool m_force_at_end_conditions;
        bool m_filter_repeated_states;
        Plan* m_initial_plan;
        TState* m_initial_state;
        std::vector<SASAction*> m_til_actions;
        TaskPlannerBase* m_planner;
        float m_timeout;
        clock_t m_initial_time;

        void createInitialPlan();
        SASAction* createInitialAction();
        SASAction* createFictitiousAction(float actionDuration,
                                          std::vector<unsigned int>& varList,
                                          float timePoint,
                                          std::string name,
                                          bool isTIL);
        Plan* createTILactions(Plan* parentPlan);
        bool checkForceAtEndConditions();  // Check if it's required to leave at-end conditions not supported for some
                                           // actions
        bool checkRepeatedStates();
        void checkPlannerType();

       public:
        TaskPlannerSetting(SASTask* sTask, bool m_generate_trace, float m_timeout);
        Plan* plan();
        Plan* improveSolution(uint16_t bestG, float bestGC, bool first);
        unsigned int getExpandedNodes();
        std::string planToPDDL(Plan* p);
    };
}  // namespace grstaps
#endif
