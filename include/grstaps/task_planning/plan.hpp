#ifndef GRSTAPS_PLAN_HPP
#define GRSTAPS_PLAN_HPP

#include "grstaps/task_planning/causal_link.hpp"
#include "grstaps/task_planning/sas_task.hpp"
#include "grstaps/task_planning/utils.hpp"

namespace grstaps
{
    class TOpenCond
    {
    public:
        TStep step;
        uint16_t condNumber;

        TOpenCond(TStep s, uint16_t c);
    };

    class Plan
    {
    private:
        void clearChildren();

    public:
        Plan* parentPlan;                        // Pointer to its parent plan
        std::vector<Plan*>* childPlans;            // Vector of child plans. This vector is nullptr if
        // the plan has not been expanded yet
        SASAction* action;                        // New action added
        float fixedEnd;                            // Fixed time for the end of the action. If the action is not fixed this value is -1
        std::vector<TOrdering> orderings;        // New orderings (first time point [lower 16 bits] -> second time point [higher 16 bits])
        std::vector<CausalLink> causalLinks;    // New causal links
        std::vector<TOpenCond>* openCond;        // Vector of open conditions (nullptr if all conditions are supported)
        bool unsatisfiedNumericConditions;
        bool repeatedState;
        float gc;
        float h;
        float hAux;
        uint16_t hLand;
        uint16_t g;
        uint32_t id;
        bool task_allocatable;

        Plan(SASAction* action, Plan* parentPlan, uint32_t idPlan);

        Plan(SASAction* action, Plan* parentPlan, float fixedEnd, uint32_t idPlan);

        ~Plan();

        void addChildren(std::vector<Plan*>& suc);

        int compare(Plan* p, int queue);

        std::string toString();

        inline bool expanded()
        {
            return childPlans != nullptr;
        }

        inline bool hasOpenConditions()
        {
            return openCond != nullptr;
        }

        void addOpenCondition(uint16_t condNumber, uint16_t stepNumber);

        inline bool isRoot()
        {
            if(parentPlan == nullptr) return true;
            if(fixedEnd >= 0) return parentPlan->isRoot();
            else return false;
        }

        inline bool isSolution()
        {
            return action != nullptr && action->isGoal && !unsatisfiedNumericConditions;
        }

        float getH(int queue);
        //void checkUsefulPlan();
    };
}
#endif // GRSTAPS_PLAN_HPP
