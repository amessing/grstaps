#ifndef TASK_PLANNER_DEADENDS_HPP
#define TASK_PLANNER_DEADENDS_HPP

#include "grstaps/task_planning/task_planner_base.hpp"

namespace grstaps
{
    class TaskPlannerDeadends : public TaskPlannerBase
    {
       private:
        Plan* initialPlan;
        Selector *selA, *selB, *currentSelector, *otherSelector;
        bool currentSelectorA;
        Plan *bestPlanA, *bestPlanB;

        void addInitialPlansToSelectors();
        void setCurrentSelector();
        bool expandBasePlan(Plan* base);
        void addSuccessors(Plan* base);

       public:
        TaskPlannerDeadends(SASTask* task,
                            Plan* initialPlan,
                            TState* initialState,
                            bool forceAtEndConditions,
                            bool filterRepeatedStates,
                            bool generateTrace,
                            std::vector<SASAction*>* tilActions,
                            TaskPlannerBase* parentPlanner,
                            float timeout);
        Plan* plan() override;
        Plan* searchStep() override;
        bool emptySearchSpace() override;
        Plan* poll() override;
        void update(Plan* base, std::vector<Plan*>& successors) override;
        std::vector<Plan*> getNextSuccessors(Plan* base) override;
    };
}  // namespace grstaps
#endif  // TASK_PLANNER_DEADENDS_HPP
