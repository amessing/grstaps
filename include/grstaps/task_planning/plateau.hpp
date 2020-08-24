#ifndef PLATEAU_HPP
#define PLATEAU_HPP

#include "grstaps/task_planning/plan.hpp"
#include "grstaps/task_planning/sas_task.hpp"
#include "grstaps/task_planning/selector.hpp"
#include "grstaps/task_planning/state.hpp"
#include "grstaps/task_planning/successors.hpp"

namespace grstaps
{
    class Plateau
    {
       private:
        SASTask* task;
        Plan* initialPlan;
        Successors* successors;
        float hToImprove;
        PlateauSelector* selector;
        std::vector<Plan*> suc;
        int selectorIndex;
        Plan* bestPlan;
        std::vector<TVarValue> priorityGoals;

        void addOpenNodes(Plan* p);
        void calculatePriorityGoals();

       public:
        Plateau(SASTask* sTask, Plan* initPlan, Successors* s, float h, int selectorIndex);
        ~Plateau();
        bool searchStep(bool concurrent);
        inline Plan* getBestPlan()
        {
            return bestPlan;
        }
        void exportOpenNodes(Selector* s)
        {
            selector->exportTo(s);
        }
        inline bool empty()
        {
            return selector->size() == 0;
        }
        inline int getSelectorIndex()
        {
            return selectorIndex;
        }
    };
}  // namespace grstaps
#endif  // PLATEAU_HPP
