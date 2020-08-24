#ifndef TASK_PLANNER_REVERSIBLE_HPP
#define TASK_PLANNER_REVERSIBLE_HPP

#include "grstaps/task_planning/task_planner_base.hpp"

namespace grstaps
{
    class TaskPlannerReversible : public TaskPlannerBase
    {
       protected:
        Plan* initialPlan;
        Selector* selector;
        Plateau* plateau;
        Plan* bestPlan;
        Plan* base;

        void addInitialPlansToSelectors();
        bool expandBasePlan(Plan* base);
        void addSuccessors(Plan* base);
        void cancelPlateauSearch(bool improve);
        void checkPlateau();

       public:
        TaskPlannerReversible(SASTask* task,
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
        virtual bool emptySearchSpace() override;
        virtual void update(Plan* base, std::vector<Plan*>& successors) override;
        virtual std::vector<Plan*> getNextSuccessors(Plan* base) override;
        virtual Plan* poll() override;
    };
}  // namespace grstaps
#endif  // TASK_PLANNER_REVERSIBLE_HPP
