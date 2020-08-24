#ifndef TASK_PLANNER_BASE_HPP
#define TASK_PLANNER_BASE_HPP

#include <time.h>

#include "grstaps/task_planning/plan.hpp"
#include "grstaps/task_planning/plateau.hpp"
#include "grstaps/task_planning/sas_task.hpp"
#include "grstaps/task_planning/selector.hpp"
#include "grstaps/task_planning/state.hpp"
#include "grstaps/task_planning/successors.hpp"

namespace grstaps
{
    class TILAction
    {
       public:
        double time;
        SASAction* action;

        TILAction(double t, SASAction* a)
        {
            time   = t;
            action = a;
        }
        bool operator<(const TILAction& a)
        {
            return time < a.time;
        }
    };

    class TaskPlannerBase
    {
       protected:
        SASTask* task;
        Plan* initialPlan;
        TState* initialState;
        bool forceAtEndConditions;
        bool filterRepeatedStates;
        bool generateTrace;
        TaskPlannerBase* parentPlanner;
        unsigned int expandedNodes;
        Successors* successors;
        std::vector<SASAction*>* tilActions;
        float initialH;
        Plan* solution;
        std::vector<Plan*> sucPlans;
        bool concurrentExpansion;
        QualitySelector qualitySelector;
        float timeout;
        clock_t startTime;

        void writeTrace(std::ofstream& f, Plan* p);
        Plan* createInitialPlan(TState* s);
        void addFrontierNodes(Plan* p);
        void calculateDeadlines();
        void updateState(TState* state, SASAction* a);
        bool timeExceed();

       public:
        TaskPlannerBase(SASTask* task,
                        Plan* initialPlan,
                        TState* initialState,
                        bool forceAtEndConditions,
                        bool filterRepeatedStates,
                        bool generateTrace,
                        std::vector<SASAction*>* tilActions,
                        TaskPlannerBase* parentPlanner,
                        float timeout);
        virtual ~TaskPlannerBase();
        virtual Plan* plan() = 0;
        std::string planToPDDL(Plan* p);
        virtual Plan* searchStep()                                      = 0;
        virtual std::vector<Plan*> getNextSuccessors(Plan* base)        = 0;
        virtual void update(Plan* base, std::vector<Plan*>& successors) = 0;
        virtual bool emptySearchSpace()                                 = 0;
        virtual Plan* poll()                                            = 0;
        unsigned int getExpandedNodes()
        {
            return expandedNodes;
        }
        Plan* improveSolution(uint16_t bestG, float bestGC, bool first);
    };
}  // namespace grstaps
#endif  // TASK_PLANNER_BASE_HPP
