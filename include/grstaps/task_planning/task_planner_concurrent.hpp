#ifndef TASK_PLANNER_CONCURRENT_HPP
#define TASK_PLANNER_CONCURRENT_HPP

#include "grstaps/task_planning/task_planner_base.hpp"

namespace grstaps
{
    class TaskPlannerConcurrent : public TaskPlannerBase
    {
       private:
        Plan* initialPlan;
        Selector* sel;
        Plateau* plateau;
        Plan* bestPlan;

        void addInitialPlansToSelectors();
        bool expandBasePlan(Plan* base);
        void addSuccessors(Plan* base);
        void cancelPlateauSearch(bool improve);
        void checkPlateau();

       public:
        TaskPlannerConcurrent(SASTask* task,
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
        void update(Plan* base, std::vector<Plan*>& successors) override;
        std::vector<Plan*> getNextSuccessors(Plan* base) override;
        Plan* poll() override;
    };
}  // namespace grstaps

#endif  // TASK_PLANNER_CONCURRENT_HPP
